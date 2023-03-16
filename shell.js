/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */
#if STRICT_JS
"use strict";

#endif
// The Module object: Our interface to the outside world. We import
// and export values on it. There are various ways Module can be used:
// 1. Not defined. We create it here
// 2. A function parameter, function(Module) { ..generated code.. }
// 3. pre-run appended it, var Module = {}; ..generated code..
// 4. External script tag defines var Module.
// We need to check if Module already exists (e.g. case 3 above).
// Substitution will be replaced with actual code on later stage of the build,
// this way Closure Compiler will not mangle it (e.g. case 4. above).
// Note that if you want to run closure, and also to use Module
// after the generated code, you will need to define   var Module = {};
// before the code. Then that object will be used in the code, and you
// can continue to use Module afterwards as well.
#if USE_CLOSURE_COMPILER
// if (!Module)` is crucial for Closure Compiler here as it will otherwise replace every `Module` occurrence with a string
var /** @type {{
  noImageDecoding: boolean,
  noAudioDecoding: boolean,
  noWasmDecoding: boolean,
  canvas: HTMLCanvasElement,
  dataFileDownloads: Object,
  preloadResults: Object,
  useWebGL: boolean,
  expectedDataFileDownloads: number,
}}
 */ Module;
if (!Module) /** @suppress{checkTypes}*/Module = {"__EMSCRIPTEN_PRIVATE_MODULE_EXPORT_NAME_SUBSTITUTION__":1};
#elif AUDIO_WORKLET
var Module = globalThis.Module || (typeof {{{ EXPORT_NAME }}} != 'undefined' ? {{{ EXPORT_NAME }}} : {});
#else
var Module = typeof {{{ EXPORT_NAME }}} != 'undefined' ? {{{ EXPORT_NAME }}} : {};
#endif // USE_CLOSURE_COMPILER

#if POLYFILL
#if ((MAYBE_WASM2JS && WASM != 2) || MODULARIZE) && (MIN_CHROME_VERSION < 33 || MIN_EDGE_VERSION < 12 || MIN_FIREFOX_VERSION < 29 || MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION < 80000)
// Include a Promise polyfill for legacy browsers. This is needed either for
// wasm2js, where we polyfill the wasm API which needs Promises, or when using
// modularize which creates a Promise for when the module is ready.
// See https://caniuse.com/#feat=promises
#include "polyfill/promise.js"
#endif

#if MIN_CHROME_VERSION < 45 || MIN_EDGE_VERSION < 12 || MIN_FIREFOX_VERSION < 34 || MIN_IE_VERSION != TARGET_NOT_SUPPORTED || MIN_SAFARI_VERSION < 90000
// See https://caniuse.com/mdn-javascript_builtins_object_assign
#include "polyfill/objassign.js"
#endif

#if WASM_BIGINT && MIN_SAFARI_VERSION < 150000
// See https://caniuse.com/mdn-javascript_builtins_bigint64array
#include "polyfill/bigint64array.js"
#endif
#endif // POLYFILL

#if MODULARIZE
// Set up the promise that indicates the Module is initialized
var readyPromiseResolve, readyPromiseReject;
Module['ready'] = new Promise(function(resolve, reject) {
  readyPromiseResolve = resolve;
  readyPromiseReject = reject;
});
#if ASSERTIONS
{{{ addReadyPromiseAssertions("Module['ready']") }}}
#endif
#endif

// --pre-jses are emitted after the Module integration code, so that they can
// refer to Module (if they choose; they can also define Module)
{{{ preJS() }}}

// Sometimes an existing Module object exists with properties
// meant to overwrite the default module functionality. Here
// we collect those properties and reapply _after_ we configure
// the current environment's defaults to avoid having to be so
// defensive during initialization.
var moduleOverrides = Object.assign({}, Module);

var arguments_ = [];
var thisProgram = './this.program';
var quit_ = (status, toThrow) => {
  throw toThrow;
};

// Determine the runtime environment we are in. You can customize this by
// setting the ENVIRONMENT setting at compile time (see settings.js).

#if AUDIO_WORKLET
var ENVIRONMENT_IS_AUDIO_WORKLET = typeof AudioWorkletGlobalScope !== 'undefined';
#endif

#if ENVIRONMENT && !ENVIRONMENT.includes(',')
var ENVIRONMENT_IS_WEB = {{{ ENVIRONMENT === 'web' }}};
#if USE_PTHREADS && ENVIRONMENT_MAY_BE_NODE
// node+pthreads always supports workers; detect which we are at runtime
var ENVIRONMENT_IS_WORKER = typeof importScripts == 'function';
#else
var ENVIRONMENT_IS_WORKER = {{{ ENVIRONMENT === 'worker' }}};
#endif
var ENVIRONMENT_IS_NODE = {{{ ENVIRONMENT === 'node' }}};
var ENVIRONMENT_IS_SHELL = {{{ ENVIRONMENT === 'shell' }}};
#else // ENVIRONMENT
// Attempt to auto-detect the environment
var ENVIRONMENT_IS_WEB = typeof window == 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts == 'function';
// N.b. Electron.js environment is simultaneously a NODE-environment, but
// also a web environment.
var ENVIRONMENT_IS_NODE = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string';
#if AUDIO_WORKLET
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER && !ENVIRONMENT_IS_AUDIO_WORKLET;
#else
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;
#endif
#endif // ENVIRONMENT

#if ASSERTIONS
if (Module['ENVIRONMENT']) {
  throw new Error('Module.ENVIRONMENT has been deprecated. To force the environment, use the ENVIRONMENT compile-time option (for example, -sENVIRONMENT=web or -sENVIRONMENT=node)');
}
#endif

#if USE_PTHREADS
// Three configurations we can be running in:
// 1) We could be the application main() thread running in the main JS UI thread. (ENVIRONMENT_IS_WORKER == false and ENVIRONMENT_IS_PTHREAD == false)
// 2) We could be the application main() thread proxied to worker. (with Emscripten -sPROXY_TO_WORKER) (ENVIRONMENT_IS_WORKER == true, ENVIRONMENT_IS_PTHREAD == false)
// 3) We could be an application pthread running in a worker. (ENVIRONMENT_IS_WORKER == true and ENVIRONMENT_IS_PTHREAD == true)

// ENVIRONMENT_IS_PTHREAD=true will have been preset in worker.js. Make it false in the main runtime thread.
var ENVIRONMENT_IS_PTHREAD = Module['ENVIRONMENT_IS_PTHREAD'] || false;
#endif

#if WASM_WORKERS
var ENVIRONMENT_IS_WASM_WORKER = Module['$ww'];
#endif

#if SHARED_MEMORY && !MODULARIZE
// In MODULARIZE mode _scriptDir needs to be captured already at the very top of the page immediately when the page is parsed, so it is generated there
// before the page load. In non-MODULARIZE modes generate it here.
var _scriptDir = (typeof document != 'undefined' && document.currentScript) ? document.currentScript.src : undefined;

if (ENVIRONMENT_IS_WORKER) {
  _scriptDir = self.location.href;
}
#if ENVIRONMENT_MAY_BE_NODE
else if (ENVIRONMENT_IS_NODE) {
  _scriptDir = __filename;
}
#endif // ENVIRONMENT_MAY_BE_NODE
#endif // SHARED_MEMORY && !MODULARIZE

// `/` should be present at the end if `scriptDirectory` is not empty
var scriptDirectory = '';
function locateFile(path) {
#if expectToReceiveOnModule('locateFile')
  if (Module['locateFile']) {
    return Module['locateFile'](path, scriptDirectory);
  }
#endif
  return scriptDirectory + path;
}

// Hooks that are implemented differently in different runtime environments.
var read_,
    readAsync,
    readBinary,
    setWindowTitle;

#if ENVIRONMENT_MAY_BE_NODE
if (ENVIRONMENT_IS_NODE) {
#if ENVIRONMENT && ASSERTIONS
  if (typeof process == 'undefined' || !process.release || process.release.name !== 'node') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif

#if ASSERTIONS
  var nodeVersion = process.versions.node;
  var numericVersion = nodeVersion.split('.').slice(0, 3);
  numericVersion = (numericVersion[0] * 10000) + (numericVersion[1] * 100) + numericVersion[2] * 1;
  var minVersion = {{{ MIN_NODE_VERSION }}};
  if (numericVersion < {{{ MIN_NODE_VERSION }}}) {
    throw new Error('This emscripten-generated code requires node {{{ formattedMinNodeVersion() }}} (detected v' + nodeVersion + ')');
  }
#endif

  // `require()` is no-op in an ESM module, use `createRequire()` to construct
  // the require()` function.  This is only necessary for multi-environment
  // builds, `-sENVIRONMENT=node` emits a static import declaration instead.
  // TODO: Swap all `require()`'s with `import()`'s?
#if EXPORT_ES6 && ENVIRONMENT_MAY_BE_WEB
  const { createRequire } = await import('module');
  /** @suppress{duplicate} */
  var require = createRequire(import.meta.url);
#endif
  // These modules will usually be used on Node.js. Load them eagerly to avoid
  // the complexity of lazy-loading.
  var fs = require('fs');
  var nodePath = require('path');

  if (ENVIRONMENT_IS_WORKER) {
    scriptDirectory = nodePath.dirname(scriptDirectory) + '/';
  } else {
#if EXPORT_ES6
    // EXPORT_ES6 + ENVIRONMENT_IS_NODE always requires use of import.meta.url,
    // since there's no way getting the current absolute path of the module when
    // support for that is not available.
    scriptDirectory = require('url').fileURLToPath(new URL('./', import.meta.url)); // includes trailing slash
#else
    scriptDirectory = __dirname + '/';
#endif
  }

#include "node_shell_read.js"

  if (process.argv.length > 1) {
    thisProgram = process.argv[1].replace(/\\/g, '/');
  }

  arguments_ = process.argv.slice(2);

#if MODULARIZE
  // MODULARIZE will export the module in the proper place outside, we don't need to export here
#else
  if (typeof module != 'undefined') {
    module['exports'] = Module;
  }
#endif

#if NODEJS_CATCH_EXIT
  process.on('uncaughtException', function(ex) {
    // suppress ExitStatus exceptions from showing an error
#if RUNTIME_DEBUG
    dbg('node: uncaughtException: ' + ex)
#endif
    if (ex !== 'unwind' && !(ex instanceof ExitStatus) && !(ex.context instanceof ExitStatus)) {
      throw ex;
    }
  });
#endif

#if NODEJS_CATCH_REJECTION
  // Without this older versions of node (< v15) will log unhandled rejections
  // but return 0, which is not normally the desired behaviour.  This is
  // not be needed with node v15 and about because it is now the default
  // behaviour:
  // See https://nodejs.org/api/cli.html#cli_unhandled_rejections_mode
  var nodeMajor = process.versions.node.split(".")[0];
  if (nodeMajor < 15) {
    process.on('unhandledRejection', function(reason) { throw reason; });
  }
#endif

  quit_ = (status, toThrow) => {
    process.exitCode = status;
    throw toThrow;
  };

  Module['inspect'] = function () { return '[Emscripten Module object]'; };

#if USE_PTHREADS
  let nodeWorkerThreads;
  try {
    nodeWorkerThreads = require('worker_threads');
  } catch (e) {
    console.error('The "worker_threads" module is not supported in this node.js build - perhaps a newer version is needed?');
    throw e;
  }
  global.Worker = nodeWorkerThreads.Worker;
#endif

#if WASM == 2
  // If target shell does not support Wasm, load the JS version of the code.
  if (typeof WebAssembly == 'undefined') {
    eval(fs.readFileSync(locateFile('{{{ TARGET_BASENAME }}}.wasm.js'))+'');
  }
#endif

} else
#endif // ENVIRONMENT_MAY_BE_NODE
#if ENVIRONMENT_MAY_BE_SHELL || ASSERTIONS
if (ENVIRONMENT_IS_SHELL) {

#if ENVIRONMENT && ASSERTIONS
  if ((typeof process == 'object' && typeof require === 'function') || typeof window == 'object' || typeof importScripts == 'function') throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif

  if (typeof read != 'undefined') {
    read_ = function shell_read(f) {
#if SUPPORT_BASE64_EMBEDDING
      const data = tryParseAsDataURI(f);
      if (data) {
        return intArrayToString(data);
      }
#endif
      return read(f);
    };
  }

  readBinary = function readBinary(f) {
    let data;
#if SUPPORT_BASE64_EMBEDDING
    data = tryParseAsDataURI(f);
    if (data) {
      return data;
    }
#endif
    if (typeof readbuffer == 'function') {
      return new Uint8Array(readbuffer(f));
    }
    data = read(f, 'binary');
    assert(typeof data == 'object');
    return data;
  };

  readAsync = function readAsync(f, onload, onerror) {
    setTimeout(() => onload(readBinary(f)), 0);
  };

  if (typeof clearTimeout == 'undefined') {
    globalThis.clearTimeout = (id) => {};
  }

  if (typeof scriptArgs != 'undefined') {
    arguments_ = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    arguments_ = arguments;
  }

  if (typeof quit == 'function') {
    quit_ = (status, toThrow) => {
      // Unlike node which has process.exitCode, d8 has no such mechanism. So we
      // have no way to set the exit code and then let the program exit with
      // that code when it naturally stops running (say, when all setTimeouts
      // have completed). For that reason, we must call `quit` - the only way to
      // set the exit code - but quit also halts immediately.  To increase
      // consistency with node (and the web) we schedule the actual quit call
      // using a setTimeout to give the current stack and any exception handlers
      // a chance to run.  This enables features such as addOnPostRun (which
      // expected to be able to run code after main returns).
      setTimeout(() => {
        if (!(toThrow instanceof ExitStatus)) {
          let toLog = toThrow;
          if (toThrow && typeof toThrow == 'object' && toThrow.stack) {
            toLog = [toThrow, toThrow.stack];
          }
          err('exiting due to exception: ' + toLog);
        }
        quit(status);
      });
      throw toThrow;
    };
  }

  if (typeof print != 'undefined') {
    // Prefer to use print/printErr where they exist, as they usually work better.
    if (typeof console == 'undefined') console = /** @type{!Console} */({});
    console.log = /** @type{!function(this:Console, ...*): undefined} */ (print);
    console.warn = console.error = /** @type{!function(this:Console, ...*): undefined} */ (typeof printErr != 'undefined' ? printErr : print);
  }

#if WASM == 2
  // If target shell does not support Wasm, load the JS version of the code.
  if (typeof WebAssembly == 'undefined') {
    eval(read(locateFile('{{{ TARGET_BASENAME }}}.wasm.js'))+'');
  }
#endif

} else
#endif // ENVIRONMENT_MAY_BE_SHELL

// Note that this includes Node.js workers when relevant (pthreads is enabled).
// Node.js workers are detected as a combination of ENVIRONMENT_IS_WORKER and
// ENVIRONMENT_IS_NODE.
#if ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  if (ENVIRONMENT_IS_WORKER) { // Check worker, not web, since window could be polyfilled
    scriptDirectory = self.location.href;
  } else if (typeof document != 'undefined' && document.currentScript) { // web
    scriptDirectory = document.currentScript.src;
  }
#if MODULARIZE
  // When MODULARIZE, this JS may be executed later, after document.currentScript
  // is gone, so we saved it, and we use it here instead of any other info.
  if (_scriptDir) {
    scriptDirectory = _scriptDir;
  }
#endif
  // blob urls look like blob:http://site.com/etc/etc and we cannot infer anything from them.
  // otherwise, slice off the final part of the url to find the script directory.
  // if scriptDirectory does not contain a slash, lastIndexOf will return -1,
  // and scriptDirectory will correctly be replaced with an empty string.
  // If scriptDirectory contains a query (starting with ?) or a fragment (starting with #),
  // they are removed because they could contain a slash.
  if (scriptDirectory.indexOf('blob:') !== 0) {
    scriptDirectory = scriptDirectory.substr(0, scriptDirectory.replace(/[?#].*/, "").lastIndexOf('/')+1);
  } else {
    scriptDirectory = '';
  }

#if ENVIRONMENT && ASSERTIONS
  if (!(typeof window == 'object' || typeof importScripts == 'function')) throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
#endif

  // Differentiate the Web Worker from the Node Worker case, as reading must
  // be done differently.
#if USE_PTHREADS && ENVIRONMENT_MAY_BE_NODE
  if (!ENVIRONMENT_IS_NODE)
#endif
  {
#include "web_or_worker_shell_read.js"
  }

  setWindowTitle = (title) => document.title = title;
} else
#endif // ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER
#if AUDIO_WORKLET && ASSERTIONS
if (!ENVIRONMENT_IS_AUDIO_WORKLET)
#endif
{
#if ASSERTIONS
  throw new Error('environment detection error');
#endif // ASSERTIONS
}

#if ENVIRONMENT_MAY_BE_NODE && USE_PTHREADS
if (ENVIRONMENT_IS_NODE) {
  // Polyfill the performance object, which emscripten pthreads support
  // depends on for good timing.
  if (typeof performance == 'undefined') {
    global.performance = require('perf_hooks').performance;
  }
}

// Set up the out() and err() hooks, which are how we can print to stdout or
// stderr, respectively.
// Normally just binding console.log/console.warn here works fine, but
// under node (with workers) we see missing/out-of-order messages so route
// directly to stdout and stderr.
// See https://github.com/emscripten-core/emscripten/issues/14804
var defaultPrint = console.log.bind(console);
var defaultPrintErr = console.warn.bind(console);
if (ENVIRONMENT_IS_NODE) {
  defaultPrint = (str) => fs.writeSync(1, str + '\n');
  defaultPrintErr = (str) => fs.writeSync(2, str + '\n');
}
{{{ makeModuleReceiveWithVar('out', 'print',    'defaultPrint',    true) }}}
{{{ makeModuleReceiveWithVar('err', 'printErr', 'defaultPrintErr', true) }}}
#else
{{{ makeModuleReceiveWithVar('out', 'print',    'console.log.bind(console)',  true) }}}
{{{ makeModuleReceiveWithVar('err', 'printErr', 'console.warn.bind(console)', true) }}}
#endif

// Merge back in the overrides
Object.assign(Module, moduleOverrides);
// Free the object hierarchy contained in the overrides, this lets the GC
// reclaim data used e.g. in memoryInitializerRequest, which is a large typed array.
moduleOverrides = null;
#if ASSERTIONS
checkIncomingModuleAPI();
#endif

// Emit code to handle expected values on the Module object. This applies Module.x
// to the proper local x. This has two benefits: first, we only emit it if it is
// expected to arrive, and second, by using a local everywhere else that can be
// minified.
{{{ makeModuleReceive('arguments_', 'arguments') }}}
{{{ makeModuleReceive('thisProgram') }}}
{{{ makeModuleReceive('quit_', 'quit') }}}

// perform assertions in shell.js after we set up out() and err(), as otherwise if an assertion fails it cannot print the message
#if ASSERTIONS
// Assertions on removed incoming Module JS APIs.
assert(typeof Module['memoryInitializerPrefixURL'] == 'undefined', 'Module.memoryInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['pthreadMainPrefixURL'] == 'undefined', 'Module.pthreadMainPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['cdInitializerPrefixURL'] == 'undefined', 'Module.cdInitializerPrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['filePackagePrefixURL'] == 'undefined', 'Module.filePackagePrefixURL option was removed, use Module.locateFile instead');
assert(typeof Module['read'] == 'undefined', 'Module.read option was removed (modify read_ in JS)');
assert(typeof Module['readAsync'] == 'undefined', 'Module.readAsync option was removed (modify readAsync in JS)');
assert(typeof Module['readBinary'] == 'undefined', 'Module.readBinary option was removed (modify readBinary in JS)');
assert(typeof Module['setWindowTitle'] == 'undefined', 'Module.setWindowTitle option was removed (modify setWindowTitle in JS)');
assert(typeof Module['TOTAL_MEMORY'] == 'undefined', 'Module.TOTAL_MEMORY has been renamed Module.INITIAL_MEMORY');
{{{ makeRemovedModuleAPIAssert('read', 'read_') }}}
{{{ makeRemovedModuleAPIAssert('readAsync') }}}
{{{ makeRemovedModuleAPIAssert('readBinary') }}}
{{{ makeRemovedModuleAPIAssert('setWindowTitle') }}}
{{{ makeRemovedFSAssert('IDBFS') }}}
{{{ makeRemovedFSAssert('PROXYFS') }}}
{{{ makeRemovedFSAssert('WORKERFS') }}}
#if !NODERAWFS
{{{ makeRemovedFSAssert('NODEFS') }}}
#endif

#if USE_PTHREADS
assert(
#if AUDIO_WORKLET
  ENVIRONMENT_IS_AUDIO_WORKLET ||
#endif
  ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER || ENVIRONMENT_IS_NODE, 'Pthreads do not work in this environment yet (need Web Workers, or an alternative to them)');
#else
#endif // USE_PTHREADS

#if !ENVIRONMENT_MAY_BE_WEB
assert(!ENVIRONMENT_IS_WEB, "web environment detected but not enabled at build time.  Add 'web' to `-sENVIRONMENT` to enable.");
#endif

#if !ENVIRONMENT_MAY_BE_WORKER
assert(!ENVIRONMENT_IS_WORKER, "worker environment detected but not enabled at build time.  Add 'worker' to `-sENVIRONMENT` to enable.");
#endif

#if !ENVIRONMENT_MAY_BE_NODE
assert(!ENVIRONMENT_IS_NODE, "node environment detected but not enabled at build time.  Add 'node' to `-sENVIRONMENT` to enable.");
#endif

#if !ENVIRONMENT_MAY_BE_SHELL
assert(!ENVIRONMENT_IS_SHELL, "shell environment detected but not enabled at build time.  Add 'shell' to `-sENVIRONMENT` to enable.");
#endif

#endif // ASSERTIONS
