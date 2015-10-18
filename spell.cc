#include <spell.h>

static strings dict;
static strings dictExt;
static strings inputFiles;
static strings textExt;
static bool verbose = false;
static bool norun = false;
static bool noenv = false;
static bool hideText = false;
static bool showNearMatches = false;

static report reportMap;

const char *noMatch=RED, *nearMatch=YELLOW;

void abortProgram (const char *msgFmt, ...);

Letters::Letters(){
  punct = lower = upper = NULL;
  end = next = 0;
}

Letters::~Letters(){
  int i;
  if ( lower ) {
	for (i=0;i<26;i++) delete (lower[i]);
	delete lower;
  }
  if ( upper ) {
	for (i=0;i<26;i++) delete (upper[i]);
	delete upper;
  }
}

int Letters::bit(char l){
  if ( l >= 'a' && l <= 'z' ) return l-'a';
  if ( l >= 'A' && l <= 'Z' ) return l-'A'+26;
  
  const char *a = ALLOWED_PUNCT;
  const char *p = strchr(a,l);
  if ( p != NULL ) return ((int)(p-a)) + 52;
  
  return -1;
}

Letters *Letters::addLetter(char l, bool isEnd){
  int b = bit(l);
  if ( b == -1 ) return NULL;
  unsigned long m = (1L << b);

  if ( isEnd ) {
	end |= m;
	return NULL;
  }
  
  if ( (next & m) == 0 ) {
	next |= m;
	if ( b < 26 ) {
	  if ( lower == NULL ){
		lower = new Letters*[26];
		memset(lower,0,26*sizeof(Letters*));
	  }
	  lower[b] = new Letters();
	} else if ( b < 52 ) {
	  if ( upper == NULL ) {
		upper = new Letters*[26];
		memset(upper,0,26*sizeof(Letters*));
	  }
	  upper[b-26] = new Letters();
	} else {
	  if ( punct == NULL ) {
		int c = strlen(ALLOWED_PUNCT);
		punct = new Letters*[c];
		memset(punct,0,c*sizeof(Letters*));
	  }
	  punct[b-52] = new Letters();
	}
  }

  if ( b < 26 ) return lower[b];
  else if ( b < 52 ) return upper[b-26];
  else return punct[b-52];
}
  
bool Letters::match(const char *w, const char *e){
  int b = bit(*w);
  unsigned long int m = (1L << b);
  if ( w == e-1 ) {
	return end & m;
  }
  if ( (next & m) == 0 ) return false;
  if ( b < 26 ) return lower[b]->match(w+1,e);
  if ( b < 52 ) return upper[b-26]->match(w+1,e);
  return punct[b-52]->match(w+1,e);
}

void abortProgram (const char *msgFmt, ...)
{
  va_list args; va_start (args, msgFmt);
  vfprintf (stderr, msgFmt, args);
  if ( errno ) fprintf (stderr, " - %s\n", strerror (errno));
  else fprintf (stderr, "\n");
  exit (1);
}

void addString(strings &vec, string value){
  if ( find(vec.begin(),vec.end(),value)==vec.end()){
	vec.push_back(value);
  }
}

bool matchExt(const char *name, strings &ext){
  const char *p = strrchr(name,'.');
  if ( p == 0 ) return false;
  const char *e = p+1;
  return find(ext.begin(),ext.end(),e) != ext.end();
}

void addDictionaryWord(const char *word, Letters *tree){
  Letters *next = tree;
  while ( next && *word ) {
	next = next->addLetter(*word,word[1]==0);
	++word;
  }
}

void addFilesFromDirectory(const char *dirname,strings &to, strings &ext){
  struct stat stats;
  struct dirent dirEntry,*dirPtr;
  DIR *dirHandle = opendir(dirname);
  if ( dirHandle == 0 ) return;
  char path[PATH_MAX];

  while ( 1 ) {
	if ( readdir_r(dirHandle,&dirEntry,&dirPtr)!=0)
	  abortProgram("Error reading directory %s",dirname);
	if ( dirPtr == 0 ) break;
	const char *name = dirPtr->d_name;
	if ( *name == '.' ) continue;
	sprintf(path,"%s/%s",dirname,name);
	if (stat(path,&stats)) abortProgram("File %s",path);
	if (S_ISDIR(stats.st_mode)){
	  addFilesFromDirectory(path,to,ext);
	} else {
	  if ( matchExt(name,ext)) addString(to,path);
	}
  }
  closedir(dirHandle);
}

void expandFilenames(strings &from, strings &to, strings &ext){
  strings::iterator s;
  struct stat stats;
  for ( s=from.begin(); s!=from.end(); ++s ){
	const char *name = (*s).c_str();
	if (stat(name,&stats)) abortProgram("File %s",name);
	if (S_ISDIR(stats.st_mode)){
	  addFilesFromDirectory(name,to,ext);
	} else {
	  addString(to,*s);
	}
  }
}

void loadDictionary(string &path,Letters *tree){
  if ( norun || verbose ) cout << "Dictionary: " << path << endl;
  if ( norun ) return;
  
  char line[1024],*l,*p;
  FILE *dict = fopen(path.c_str(),"r");
  if ( dict == NULL ) abortProgram("Can't open %s",path.c_str());
  while ( fgets(line,sizeof(line),dict)==line ){
	l = line;
	while ( *l == ' ' ) l++;
	if ( (p=strchr(l,'\n')) !=0 ) *p=0;
	if ( (p=strchr(l,' ')) !=0 ) *p=0;
	if ( *l ) addDictionaryWord(l,tree);
  }
  if (!feof(dict)) abortProgram("Error reading %s",path.c_str());

  fclose(dict);
}

void loadDictionaries(strings &dictionaries, Letters *tree){
  strings::iterator s;
  for ( s=dictionaries.begin(); s!=dictionaries.end(); ++s ) loadDictionary(*s,tree);
}

string getWord(const char *w, const char *e){
  string s;
  for ( ; w < e; w++ ) s.append(1,*w);
  return s;
}

bool tryVariations(char *w, char *e, Letters *tree){
  bool m;
  if ( isupper(*w)) {
	*w = tolower(*w);
	m = tree->match(w,e);
	*w = toupper(*w);
	if ( m ) return true;
  }

  long l = ((long)e) - ((long)w);
  
  if ( w[l-1] == '\'' || w[l-1] == 's') {
	m = tree->match(w,e-1) || tryVariations(w,e-1,tree);
	if ( m ) return true;
	
  }
  
  if ( l > 2 && w[l-2]=='\'' && w[l-1]=='s' ) {
	m = tree->match(w,e-2) || tryVariations(w,e-2,tree);
	if ( m ) return true;
  }
  
  return false;
}

void checkFile(string &path, Letters *tree){
  string filepath = path;
  bool firstForFile = true;
  if ( norun ) {
	  cout << "Check file " << path << endl;
	  return;
  }
  if ( verbose == true ) {
	cout << endl << RED << path << CLEAR << endl;
	firstForFile = false;
  }
  
  char line[5000],*w,*e, *start;
  FILE *text = fopen(path.c_str(),"r");
  bool near;
  
  if ( text == NULL ) abortProgram("Can't open %s",path.c_str());
  while ( fgets(line,sizeof(line),text)==line){
	w = start = line;
	while ( *w ) {
	  while ( *w && !isalpha(*w)) {
		w++;
	  }
	  if ( *w == 0 ) continue;
	  e = w+1;
	  while ( *e && (isalpha(*e) || strchr(ALLOWED_PUNCT,*e)) ) e++;

	  near = false;
	  bool m = tree->match(w,e);

	  if ( !m ) near = tryVariations(w,e,tree);
	  
	  if ( !m && near && showNearMatches == false ) {
		// Consider near matches to be actual matches
		m = true;
		near = false;
	  }
	  
	  if ( !m && !near ) {
		if ( reportMap.find(filepath) == reportMap.end() ) {
		  strings words;
		  reportMap.insert(make_pair(filepath,words));
		}
		string wrd = getWord(w,e);
		report::iterator r = reportMap.find(filepath);
		if ( find(r->second.begin(),r->second.end(),wrd)==r->second.end() ) {
		  r->second.push_back(getWord(w,e));
		}
	  }
	  
	  if ( (!m || near) && hideText == false ) {
		if ( firstForFile ) {
		  cout << endl << RED << filepath << CLEAR << endl;
		  firstForFile = false;
		}
		while ( start < w ) putchar(*start++);
		if ( near ) {
		  printf("%s",nearMatch);
		} else {
		  printf("%s",noMatch);
		}
		printf("%s",near?nearMatch:noMatch);
		while ( start < e ) putchar(*start++);
		printf("%s",CLEAR);
	  }
	  w=e;
	}
	if ( hideText == false && start != line ) {
	  while (*start) putchar(*start++);
	}
  }
  if (!feof(text)) abortProgram("Error reading %s",path.c_str());
}

void showHelp(const char *programName){
  cout << programName << " [-v] [-x] [-d <dct>]... [-w <dict_ext>]... [-e <text_ext>]... <input>..." 
	   << endl;
  cout << "" << endl;
  cout << programName << " reads words from text files and checks them against words in ";
  cout << "dictionary files." << endl;
  cout << "" << endl;
  cout << "The following arguments may be provided:" << endl;
  cout << "" << endl;
  cout << "  -d <dictionary_name> : add a dictionary name or directory to search" << endl;
  cout << "     for directory names. If a directory name is entered files with " << endl;
  cout << "     extensions defined by -w arguments will be loaded. Dictonary names" << endl;
  cout << "     may also be defined using the " << SPELL_DCT_ENV << " environment variable." << endl;
  cout << "" << endl;
  cout << "  -w <dictonary_extension> : Specify an extension for dictionary files." << endl;
  cout << "     You may specify multiple -w arguments. This is used when a directory" << endl;
  cout << "     name is specified by a -d argument. If no extensions are provided" << endl;
  cout << "     the default dictionary extension is \"" << DEFAULT_DEXT << "\"." <<endl;
  cout << "" << endl;
  cout << "  -e <text_extension> : specify an extension for text files." << endl;
  cout << "     You may specify multiple -e arguments. This is used when a directory" << endl;
  cout << "     name is specified as an input file argument. If no extensions are provided" << endl;
  cout << "     the default text file extension is \"" << DEFAULT_EXT << "\"." << endl;
  cout << "" << endl;
  cout << "  -v : verbose mode - show dictionaries and filenames as they are processed." << endl;
  cout << "" << endl;
  cout << "  -x : no run - show which dictionary and input files will be processed." << endl;
  cout << "" << endl;
  cout << "  -c : no env - do not load dictionaries from " << SPELL_DCT_ENV << endl;
  cout << "" << endl;
  cout << "  -t : do not show lines containing mispelled words - no text" << endl;
  cout << "" << endl;
  cout << "  -n : Show near matches - words that are similar but not exactly in a dirctionary." << endl;
  cout << "" << endl;
  cout << "  -h or -?: show this help" << endl;
  cout << "" << endl;
  cout << "  input : All arguments following flags are taken as input files to process." << endl;
  cout << "" << endl;
  cout << "The " SPELL_DCT_ENV << " environment variable can be used to specify which" << endl;
  cout << "dictionary files to read. Dictionary files and directories are separated by " << endl;
  cout << "colon characters." << endl;
  cout << "" << endl;
  
  const char *dctList = getenv(SPELL_DCT_ENV);
  if ( dctList && &dctList ) {
	cout << "The current value of " << SPELL_DCT_ENV << " is \"" << dctList << "\"" << endl;
  } else {
	cout << "The environment variable " << SPELL_DCT_ENV << " is not currently set." << endl;
	cout << "You may want to set it like this:" << endl;
	cout << "" << endl;
	cout << "   export " << SPELL_DCT_ENV << "=\"/usr/share/dict/words:.\"" << endl;
  }
  cout << "" << endl;
  cout << "Words must start with a letter and may only contain letters or the following" << endl;
  cout << "other characters: " << ALLOWED_PUNCT << endl;
}

void checkFiles(strings &filesToCheck, Letters *tree){
  strings::iterator s;
  for (s=filesToCheck.begin();s!=filesToCheck.end();++s) checkFile(*s, tree);
}

void getArgs(int argc, char *argv[]){
  int c,i;
  while ((c=getopt(argc,argv,"?d:e:w:vxctnh")) != -1){
	switch (c){
	case 'd':
	  addString(dict,optarg);
	  break;
	case 'e':
	  addString(textExt,optarg);
	  break;
	case 'w':
	  addString(dictExt,optarg);
	  break;
	case 'v':
	  verbose = true;
	  break;
	case 'x':
	  norun = true;
	  break;
	case 'c':
	  noenv = true;
	  break;
	case 't':
	  hideText = true;
	  break;
	case 'n':
	  showNearMatches = true;
	  break;
	case 'h':
	  showHelp(argv[0]);
	  abortProgram("");	  
	case '?':
	  showHelp(argv[0]);
	  abortProgram("");
	}
  }

  if ( !noenv ) {
	const char *dctList = getenv(SPELL_DCT_ENV);
	if ( dctList && &dctList ) {
	  const char *sep = ":";
	  char *tmp = strdup (dctList);
	  char *tok = strtok (tmp, sep);
	  while ( tok )
	  {
		dict.push_back(tok);
		tok = strtok (0, sep);
	  }
	  free (tmp);
	}
  }
  
  for ( i=optind; i < argc; i++ ) {
	inputFiles.push_back(string(argv[i]));
  }

  if ( inputFiles.size() == 0 ) inputFiles.push_back(DEFAULT_FILE);
  if ( textExt.size() == 0 ) textExt.push_back(DEFAULT_EXT);
  if ( dictExt.size() == 0 ) dictExt.push_back(DEFAULT_DEXT);
}

int main (int argc, char *argv[]){
  static Letters tree;
  static strings dictToLoad;
  static strings filesToCheck;
  
  getArgs(argc,argv);

  expandFilenames(dict,dictToLoad,dictExt);
  if ( dict.size() == 0 ) {
	cout << "No dictionaries. Use -d or set " << SPELL_DCT_ENV << endl;
	cout << argv[0] << " -? for help" << endl;
	return 0;
  }

  expandFilenames(inputFiles,filesToCheck,textExt);
  if ( filesToCheck.size() == 0 ) {
	cout << "No files to check" << endl;
	cout << argv[0] << " -? for help" << endl;
	return 0;
  }
  
  memset(&tree,0,sizeof(Letters));
  loadDictionaries(dictToLoad, &tree);
  checkFiles(filesToCheck, &tree);
  
  cout << endl;
  report::iterator r;
  for ( r=reportMap.begin();r!=reportMap.end();++r){
	cerr << r->first << ": " << noMatch;
	strings::iterator s;
	for ( s=r->second.begin();s!=r->second.end();++s) {
	  cerr << *s << " ";
	}
	cerr << CLEAR << endl;
  }
  
  return 0;
}
