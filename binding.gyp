{
  "targets": [
    {
      "target_name": "wson_addon",
      "sources": [ 
        "src/parser.cc",
        "src/stringifier_target.cc",
        "src/stringifier.cc",
        "src/parser_source.cc",
        "src/wson.cc"
      ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}
