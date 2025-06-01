{
  "targets": [
    {
      "target_name": "lljs",
      "sources": [
        "src/native/memory.cpp",
        "src/native/cpu.cpp",
        "src/native/system.cpp",
        "src/native/io.cpp",
        "src/native/threading.cpp",
        "src/native/time.cpp",
        "src/native/math.cpp",
        "src/native/string.cpp",
        "src/native/lljs.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "src/native/headers"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [
        "-fno-exceptions"
      ],
      "cflags_cc!": [
        "-fno-exceptions"
      ],
      "defines": [
        "NAPI_DISABLE_CPP_EXCEPTIONS",
        "WIN32_LEAN_AND_MEAN",
        "NOMINMAX",
        "_CRT_SECURE_NO_WARNINGS"
      ],
      "conditions": [
        [
          "OS=='win'",
          {
            "defines": [
              "_CRT_SECURE_NO_WARNINGS",
              "_WIN32_WINNT=0x0600"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "ExceptionHandling": 1,
                "AdditionalOptions": [
                  "/W3"
                ]
              }
            }
          }
        ],
        [
          "OS=='mac'",
          {
            "xcode_settings": {
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "CLANG_CXX_LIBRARY": "libc++",
              "MACOSX_DEPLOYMENT_TARGET": "10.7"
            }
          }
        ]
      ]
    }
  ]
}