// Wrapper for the FreeTDS db-lib library
// -- Sugendran Ganess
//
// most of this is based on the excellent tutorials over at
// - http://syskall.com/how-to-write-your-own-native-nodejs-extension
// - https://www.cloudkick.com/blog/2010/aug/23/writing-nodejs-native-extensions/
// - http://nikhilm.bitbucket.org/articles/c_in_my_javascript/c_in_javascript_part_1.html
//
// In the end I decided to run with static functions
//  - it's just a wrapper so anything clever will need to be done in js
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <v8.h>
#include <node.h>
#include <uv.h>
#include <sqldb.h>
#include <sybdb.h>

#include "node_freetds.h"

#define ERROR(msg) \
	ThrowException(Exception::Error(String::New(msg)))

#define TYPE_ERROR(msg) \
	ThrowException(Exception::TypeError(String::New(msg)))

// handy helpers for extracting arguments
#define REQ_ARG(TYPE, I, VAR)                                           \
  if (args.Length() <= (I) || !args[I]->Is##TYPE())						\
    return TYPE_ERROR("Argument " #I " must be a " #TYPE);				\
  Local<TYPE> VAR = Local<TYPE>::Cast(args[I]);

// handy helpers for extracting arguments
#define OPT_ARG(TYPE, I, VAR)                                           \
  Local<TYPE> VAR =	Local<TYPE>::Cast(args[I]);							\
  if (args.Length() > (I) && !args[I]->Is##TYPE())						\
    return TYPE_ERROR("Argument " #I " must be a " #TYPE);				\

#define REQ_DBCONN_ARG(I)												\
		DBPROCESS *dbconn = (DBPROCESS*) External::Unwrap(args[I]);

using namespace v8;
using namespace node;

void FreeTDS::Initialize(Handle<Object> target) {

	if(dbinit() == FAIL) {
		ERROR("Cannot initialize freetds.  Do you have it installed - libfreetds?");
		return;
	}

	Local<FunctionTemplate> t = FunctionTemplate::New(FreeTDS::New);

	t->InstanceTemplate()->SetInternalFieldCount(1);
	t->SetClassName(String::NewSymbol("FreeTDS"));

	t->InstanceTemplate()->SetAccessor(String::New("server"), getServer);
	t->InstanceTemplate()->SetAccessor(String::New("database"), getDatabase);

	NODE_SET_PROTOTYPE_METHOD(t, "version", FreeTDS::Version);
	NODE_SET_PROTOTYPE_METHOD(t, "login", FreeTDS::Login);
	NODE_SET_PROTOTYPE_METHOD(t, "logout", FreeTDS::Logout);
	NODE_SET_PROTOTYPE_METHOD(t, "cleanup", FreeTDS::Cleanup);
	NODE_SET_PROTOTYPE_METHOD(t, "executeSql", FreeTDS::ExecuteStatement);

	target->Set(String::NewSymbol("FreeTDS"), t->GetFunction());
}

Handle<Value> FreeTDS::New(const Arguments& args) {
  HandleScope scope(node_isolate);
  REQ_ARG(String, 0, server);
  REQ_ARG(String, 1, database);

  FreeTDS *p = new FreeTDS();
  p->Wrap(args.This());

  // TODO Free these 2!
  if(!asprintf(&p->server, "%s", *(String::Utf8Value(server))))
	  return ERROR("Out of memory");

  if(!asprintf(&p->database, "%s", *(String::Utf8Value(database))))
	  return ERROR("Out of memory");

  return args.This();
}

Handle<Value> FreeTDS::Version(const Arguments& args) {
  return String::New(rcsid_sqldb_h);
}

/*
 * Try: user, password[, callback]
 * returns true if login was successful
 */
Handle<Value> FreeTDS::Login(const Arguments& args) {
  HandleScope scope(node_isolate);
  FreeTDS *freetds = FreeTDS::Unwrap(args);
  LOGINREC *login;

  REQ_ARG(String, 0, user);
  REQ_ARG(String, 1, password);
  // TODO Implement!
  OPT_ARG(Object, 2, callback);

  // TODO Do this in another thread if a callback is supplied!
  // TODO If an execute is called before this is completed then use `join'
  // Allocate a login params structure
  if ((login = dblogin()) == FAIL)
	return ERROR("FreeTDS cannot initialize dblogin() structure.");

  // fill out the login params
  // TODO Do this without creating a new AsciiValue...
  String::AsciiValue userId(user);
  DBSETLUSER(login, *userId);
  // TODO Do this without creating a new AsciiValue...
  String::AsciiValue pw(password);
  DBSETLPWD(login, *pw);

  DBSETLVERSION(login, DBVERSION_80);

  printf("User: %s\n", *userId);
  printf("Password: %s\n", *pw);

  // set the application name to node-freetds to help with profiling etc
  DBSETLAPP(login, "node-freetds");

  // Now connect to the DB Server
  if ((freetds->dbconn = dbopen(login, freetds->server)) == NULL) {
	dbloginfree(login);
	return ERROR("FreeTDS cannot initialize dblogin() structure.");
  }

  // TODO Fails for some reason... maybe add exception handling?
  // Now switch to the correct database
  /*
  if ((dbuse(freetds->dbconn, freetds->database)) == FAIL) {
	dbloginfree(login);
	dbclose(freetds->dbconn);
	return ERROR("FreeTDS could not switch to the database");
  }
  //*/
  // free the login struct because we don't need it anymore
  dbloginfree(login);

  // wrap the dbconn so that we can persist it from the JS side
  return True(node_isolate);
}

// one arg - logout
Handle<Value> FreeTDS::Logout(const Arguments& args) {
  HandleScope scope(node_isolate);
  FreeTDS *freetds = FreeTDS::Unwrap(args);

  dbfreebuf(freetds->dbconn);
  dbclose(freetds->dbconn);

  return Undefined(node_isolate);
}

// TODO Logout and Cleanup should happen when the object is freed and probably should not be exposed to Javascript.
// TODO Calling dbexit here will be really confusing to freetds if it is called out of order when there is more than one dbinit or instance of FreeTDS.
Handle<Value> FreeTDS::Cleanup(const Arguments& args) {
  HandleScope scope(node_isolate);
  dbexit();
  return Undefined(node_isolate);
}

void onDataResponse(uv_work_t *req, int stat) {
  HandleScope scope(node_isolate);
  FreeTDS::data_callback_t *callbackData = static_cast<FreeTDS::data_callback_t*>(req->data);

  if(callbackData->response == FAIL){
	char *err;
	// TODO This creates an error :-(
	Local<Value> argv[1];

	asprintf(&err,
			"An error occurred executing statement: \"%s\"",
			callbackData->statement);

	argv[0] = Exception::Error(String::New(err));
	callbackData->callback->Call(Context::GetCurrent()->Global(), 1, argv);

	callbackData->callback.Dispose();
	delete[] err;
	delete callbackData;

	return;
  }

  uint32_t rownum = 0;
  bool err = false;
  struct FreeTDS::COL *columns, *pcol;
  int ncols = 0;

  //Persistent<Context> c = Context::New();
  //c->Global()->Set(String::New("test"), String::New("results"));

  Handle<Value> argv[1];
  Local<Array> results = Array::New();

  while(dbresults(callbackData->dbconn) != NO_MORE_RESULTS) {
	ncols = dbnumcols(callbackData->dbconn);
	columns = (struct FreeTDS::COL *) calloc(ncols, sizeof(struct FreeTDS::COL));
	for (pcol = columns; pcol - columns < ncols; pcol++) {
	  int i = pcol - columns + 1;
	  pcol->name = String::New(dbcolname(callbackData->dbconn, i));
	  pcol->type = dbcoltype(callbackData->dbconn, i);
	  pcol->size = dbcollen(callbackData->dbconn, i);

	  if (SYBCHAR != pcol->type) {
		pcol->size = dbwillconvert(pcol->type, SYBCHAR);
	  }
	  //todo: work out if I'm leaking
	  if((pcol->buffer = (void *) malloc(pcol->size + 1)) == NULL) {
		err = true;
		break;
	  }
	}
	if(err){
	  for (pcol = columns; pcol - columns < ncols; pcol++) {
		free(pcol->buffer);
		free(columns);
	  }
	  Local<Value> argv[1];
	  argv[0] = Exception::Error(String::New("Could not allocate memory for columns"));
	  callbackData->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	  return;
	}
	for (pcol = columns; pcol - columns < ncols; pcol++) {
	  int i = pcol - columns + 1;
	  int binding = NTBSTRINGBIND;
	  // switch(pcol->type){
	  //   case TINYBIND:
	  //   case SMALLBIND:
	  //   case INTBIND:
	  //   case FLT8BIND:
	  //   case REALBIND:
	  //   case SMALLDATETIMEBIND:
	  //   case MONEYBIND:
	  //   case SMALLMONEYBIND:
	  //   case BINARYBIND:
	  //   case BITBIND:
	  //   case NUMERICBIND:
	  //   case DECIMALBIND:
	  //   case BIGINTBIND:
	  //     // all numbers in JS are doubles
	  //     binding = REALBIND;
	  //     break;
	  // }
	  if(dbbind(callbackData->dbconn, i, binding,  pcol->size + 1, (BYTE*)pcol->buffer) == FAIL){
		err = true;
	  }else if(dbnullbind(callbackData->dbconn, i, &pcol->status) == FAIL){
		err = true;
	  }
	}
	if(err){
	  for (pcol = columns; pcol - columns < ncols; pcol++) {
		free(pcol->buffer);
		free(columns);
	  }
	  Local<Value> argv[1];
	  argv[0] = Exception::Error(String::New("Could not allocate memory for columns"));
	  callbackData->callback->Call(Context::GetCurrent()->Global(), 1, argv);
	  return;
	}
	int row_code;

	while ((row_code = dbnextrow(callbackData->dbconn)) != NO_MORE_ROWS){
	  if(row_code == REG_ROW) {
		Local<Object> tuple = Object::New();
		for (pcol = columns; pcol - columns < ncols; pcol++) {
		  if(pcol->status == -1){
			tuple->Set(pcol->name, Null());
			continue;
		  }
		  switch(pcol->type){
			case SQLINTN:
			case SQLINT1:
			case SQLINT2:
			case SQLINT4:
			case SQLINT8:
			case SQLFLT8:
			case SQLDATETIME:
			case SQLDATETIM4:
			case SQLBIT:
			case SQLFLT4:
			case SQLNUMERIC:
			case SQLDECIMAL:
			case SQLFLTN:
			case SQLDATETIMN:
			case 36:
			case SQLCHAR:
			case SQLVARCHAR:
			case SQLTEXT:
			  tuple->Set(pcol->name, String::New((char*) pcol->buffer));
			  break;
			// case SQLINTN:
			// case SQLINT1:
			// case SQLINT2:
			// case SQLINT4:
			// case SQLINT8:
			// case SQLFLT8:
			// case SQLDATETIME:
			// case SQLDATETIM4:
			// case SQLBIT:
			// case SQLFLT4:
			// case SQLNUMERIC:
			// case SQLDECIMAL:
			// case SQLFLTN:
			// case SQLDATETIMN:
			  // DBREAL val;
			  // memcpy(&val, pcol->buffer, pcol->size);
			  // tuple->Set(pcol->name, Number::New((double)val));
			  break;
			case SQLIMAGE:
			case SQLMONEY4:
			case SQLMONEY:
			case SQLBINARY:
			case SQLVARBINARY:
			case SQLMONEYN:
			case SQLVOID:
			default:
			  // TODO This is an exception, right?
			  printf("unsupported col type %d\n", pcol->type);
			  break;
		  }
		}
		results->Set(rownum++, tuple);
	  }
	}
	for (pcol = columns; pcol - columns < ncols; pcol++) {
	  free(pcol->buffer);
	}
	free(columns);
  }

  argv[0] = results;
  callbackData->callback->Call(Context::GetCurrent()->Global(), 1, argv);

  callbackData->callback.Dispose();
  delete callbackData;

  return;
}

static void waitForDataResponse(uv_work_t *req) {
  FreeTDS::data_callback_t *callbackData = static_cast<FreeTDS::data_callback_t*>(req->data);
  callbackData->response = dbsqlok(callbackData->dbconn);
}

// db, statement, callback
Handle<Value> FreeTDS::ExecuteStatement(const Arguments& args) {
  HandleScope scope(node_isolate);
  FreeTDS *freetds = FreeTDS::Unwrap(args);

  REQ_ARG(String, 0, js_statement);
  REQ_ARG(Function, 1, js_callback);

  String::AsciiValue statement(js_statement);

  if(dbcmd(freetds->dbconn, *statement) == FAIL)
	return ERROR("FreeTDS could allocate enough memory for the statement");

  // TODO Free!
  data_callback_t *callbackData = new data_callback_t();
  memset(callbackData, 0, sizeof(data_callback_t));

  callbackData->dbconn = freetds->dbconn;
  // TODO Free!
  if(!asprintf(&callbackData->statement, "%s", *statement))
	  return ERROR("Out of memory.");;
  if(js_callback->IsFunction())
	  callbackData->callback = Persistent<Function>::New(node_isolate, js_callback);

  if(dbsqlsend(freetds->dbconn) == FAIL) {
	delete callbackData;
	return ERROR("FreeTDS could not send the statement.");
  }

  callbackData->worker.data = callbackData;

  uv_queue_work(uv_default_loop(), &callbackData->worker, waitForDataResponse,  onDataResponse);

  return Undefined(node_isolate);
}

NODE_MODULE(node_freetds, FreeTDS::Initialize);
