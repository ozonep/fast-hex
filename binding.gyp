{
  "conditions": [
    ['OS=="win"', {
      "variables": {
        "has_avx2%": "<!(.\util\cpuinfo.exe 1 5 7)"
      }
    }]
  ],
  "targets": [
    {
      "target_name": "strdecode",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "xcode_settings": { "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
        "CLANG_CXX_LIBRARY": "libc++",
        "MACOSX_DEPLOYMENT_TARGET": "10.7",
      },
      "msvs_settings": {
        "VCCLCompilerTool": { "ExceptionHandling": 1 },
      },
      "sources": [
        "src/strdecode.cc",
        "src/hex.cc"
      ],
      "include_dirs" : [
      ],
      "cflags":[
        "-march=native"
      ],
      "conditions": [
        ['OS=="win" and has_avx2==1', {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "EnableEnhancedInstructionSet": 5 # /arch:AVX2
            }
          }
        }, {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "EnableEnhancedInstructionSet": 3 # /arch:AVX
            }
          }
        }]
      ]
    }
  ]
}
