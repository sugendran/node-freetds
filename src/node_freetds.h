/*
 * node_freetds.h
 *
 *  Created on: Jun 22, 2013
 *      Author: Nathaniel "Gnate" Sigrist <njsigrist@mmm.com>
 */

#ifndef NODE_FREETDS_H_
#define NODE_FREETDS_H_

#include <v8.h>
#include <node.h>

using namespace v8;
using namespace node;

class FreeTDS : public node::ObjectWrap {
	public:

	  struct data_callback_t {
		uv_work_t worker;
	    DBPROCESS *dbconn;
	    char *statement;
	    int response;
	    Persistent<Function> callback;
	  };

	  struct COL {
	    Local<String> name;
	    void *buffer;
	    int type, size, status;
	  };

	  static FreeTDS* Unwrap(const v8::Arguments& args) {
	    return node::ObjectWrap::Unwrap<FreeTDS>(args.This());
	  }

	  static FreeTDS* Unwrap(const AccessorInfo& info) {
		return node::ObjectWrap::Unwrap<FreeTDS>(info.Holder());
	  }

	  static void Initialize(Handle<Object> target);

	protected:
	  DBPROCESS *dbconn;
	  char *server;
	  char *database;

	  static v8::Handle<v8::Value> New(const v8::Arguments& args);
	  static v8::Handle<v8::Value> Init(const v8::Arguments& args);
	  static v8::Handle<v8::Value> Close(const v8::Arguments& args);

	  static v8::Handle<v8::Value> Version(const v8::Arguments& args);
	  static v8::Handle<v8::Value> Login(const v8::Arguments& args);
	  static v8::Handle<v8::Value> Logout(const v8::Arguments& args);
	  static v8::Handle<v8::Value> Cleanup(const v8::Arguments& args);
	  static v8::Handle<v8::Value> ExecuteStatement(const v8::Arguments& args);

	  static v8::Handle<v8::Value> getServer(Local<String> property,
			  const AccessorInfo& info) {
		  v8::HandleScope scope(node_isolate);
		  FreeTDS *freetds = FreeTDS::Unwrap(info);
		  return scope.Close(v8::String::New(freetds->server));
	  }

	  static v8::Handle<v8::Value> getDatabase(Local<String> property,
              const AccessorInfo& info) {
		  v8::HandleScope scope(node_isolate);
		  FreeTDS *freetds = FreeTDS::Unwrap(info);
		  return scope.Close(v8::String::New(freetds->database));
	  }
};


#endif /* NODE_FREETDS_H_ */
