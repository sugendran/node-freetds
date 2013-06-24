{
	"targets": [ {
			"target_name": "node-freetds",
			"sources": [ "src/node_freetds.cpp" ],
			'type': 'loadable_module',
			"product_extension": "node",
			"conditions": [ [
				'OS=="linux"', {
					"libraries": [
						"$(shell find /usr/lib -type l -name 'libv8.so')",
						"$(shell find /usr/lib -type l -name 'libsybdb.so')"
					],
					"headers": [
						"$(shell find /usr/include -type f -name 'v8.h')",
						"$(shell find /usr/include -type f -name 'node.h')",
						"$(shell find /usr/include -type f -name 'uv.h')",
						"$(shell find /usr/include -type f -name 'sqldb.h')",
						"$(shell find /usr/include -type f -name 'sybdb.h')"
					]
				}], [
				'OS=="win"', {
					"include_dirs": [
						"-L.//include",
						"-L./node/src"
					]
				}
			],
			"cflags": [
				"-g",
				"-D_FILE_OFFSET_BITS=64",
				"-D_LARGEFILE_SOURCE",
				"-Wall"
			],
			"ldflags": [
				"-lpthread",
				"-lv8",
				"-lsybdb"
			]
		}
	],
}