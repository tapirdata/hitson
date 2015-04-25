{
  "targets": [
    {
      "target_name": "tson",
      "sources": [ 
        "src/parser.cc",
        "src/transcribe.cc",
        "src/stringifier_target.cc",
        "src/stringifier.cc",
        "src/parser_source.cc",
        "src/tson.cc"
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ],
      "cflags": ["-std=c++11"]
    }
  ]
}
