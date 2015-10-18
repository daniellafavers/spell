# spell

C++ spell check

spell [-v] [-x] [-d <dct>]... [-w <dict_ext>]... [-e <text_ext>]... <input>...

spell reads words from text files and checks them against words in dictionary files.

The following arguments may be provided:

  -d <dictionary_name> : add a dictionary name or directory to search
     for directory names. If a directory name is entered files with 
     extensions defined by -w arguments will be loaded. Dictonary names
     may also be defined using the SPELL_DICTIONARIES environment variable.

  -w <dictonary_extension> : Specify an extension for dictionary files.
     You may specify multiple -w arguments. This is used when a directory
     name is specified by a -d argument. If no extensions are provided
     the default dictionary extension is "dict".

  -e <text_extension> : specify an extension for text files.
     You may specify multiple -e arguments. This is used when a directory
     name is specified as an input file argument. If no extensions are provided
     the default text file extension is "txt".

  -v : verbose mode - show dictionaries and filenames as they are processed.

  -x : no run - show which dictionary and input files will be processed.

  -c : no env - do not load dictionaries from SPELL_DICTIONARIES

  -t : do not show lines containing mispelled words - no text

  -n : Show near matches - words that are similar but not exactly in a dirctionary.

  -h or -?: show this help

  input : All arguments following flags are taken as input files to process.

The SPELL_DICTIONARIES environment variable can be used to specify which
dictionary files to read. Dictionary files and directories are separated by 
colon characters.

The current value of SPELL_DICTIONARIES is "/Users/daniel/dictionaries"

Words must start with a letter and may only contain letters or the following
other characters: _'

-------

A dictionary file is a newline-separated list of words. The spell program will allow
variations, such as words with different case or words that have s or 's following the
dictionary word.

This has a simple makefile. No configure or install.

English word lists can be found at: http://wordlist.sourceforge.net/

Put these into your dictionaries directory.

This spell program will look for a list of text files in a directory tree for
each directory specified by a -d argument, or as listed in the SPELL_DICTIONARIES
environment variable.

All text files with the required extension in the directory tree rooted at the current
directory.

See examples/runme
