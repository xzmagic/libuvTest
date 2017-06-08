{
    'targets': [
      {
        'target_name': 'connect',
        'product_name': 'connect',
        'type': 'executable',
     
        'dependencies': [
          'D:/TOOL/node-v5.2.0/deps/uv/uv.gyp:libuv',
        ],

        'include_dirs': [
          'D:/TOOL/node-v5.2.0/deps/uv/include',
          "C:/msys64/MinGW/include",
          "C:/Program Files (x86)/Microsoft Visual Studio/2017/Professional/VC/Tools/MSVC/14.10.25017/include/*",
        ],

        'sources': [
          'util.h',
          'util.cpp',
          'ServerConnectionTest.cpp',
        ],

      }, 
    ]
  }