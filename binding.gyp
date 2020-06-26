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
        "-O3",
        "-Wno-narrowing",
        "-march=native"
      ],
      "cflags_cc":[
        "-O3",
        "-Wno-narrowing",
        "-march=native"
      ]
    }
  ]
}
