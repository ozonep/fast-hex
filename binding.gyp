{
  "targets": [
    {
      "target_name": "strdecode",
      "sources": [
        "src/strdecode.cc",
        "src/hex.cc"
      ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ],
      "cflags":[
        "-Ofast",
        "-Wno-cast-function-type",
        "-march=haswell"
      ],
      "cflags_cc":[
        "-Ofast",
        "-Wno-cast-function-type",
        "-march=haswell"
      ]
    }
  ]
}
