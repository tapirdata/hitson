{
  "targets": [
    {
      "target_name": "hitson",
      "sources": [ 
        "src/parser.cc",
        "src/stringifier_target.cc",
        "src/stringifier.cc",
        "src/parser_source.cc",
        "src/tson.cc"
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}
