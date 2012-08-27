{
	"targets": [ {
			"target_name": "node-freetds",
			"sources": [ "src/node_freetds.cpp" ],
			'type': 'loadable_module',
			"product_extension": "node",
			"conditions": [ [
				'OS=="linux"', {
					"libraries": [
						"$(shell find /usr/lib -type l -name 'libsybdb.so')"
					]
				}]
			],
			"cflags": [
				"-g",
				"-D_FILE_OFFSET_BITS=64",
				"-D_LARGEFILE_SOURCE",
				"-Wall"
			],
			"ldflags": [
				"-lpthread",
				"-lsybdb"
			]
		}
	],
}