"use strict";

var do_iter = true;

var Module = {
  preRun: [],
  postRun: [],
  print: (function () {
    return function (text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      console.log(text);
    };
  })(),
  printErr: function (text) {
    if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
    console.error(text);
  },
  canvas: (function () {
    var canvas = document.getElementById('canvas');

    // As a default initial behavior, pop up an alert when webgl context is lost. To make your
    // application robust, you may want to override this behavior before shipping!
    // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
    canvas.addEventListener("webglcontextlost", function (e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

    return canvas;
  })(),


  goFullscreen: function () {
    var canvas = Module['canvas'];
    if (Browser.isFullscreen) return canvas.exitFullscreen();
    Browser.lockPointer = false;
    Browser.resizeCanvas = false;
    Browser.vrDevice = null;
    function fullscreenChange() {
      Browser.isFullscreen = false;
      var canvasContainer = canvas.parentNode;
      if ((document['fullscreenElement'] || document['mozFullScreenElement'] ||
        document['msFullscreenElement'] || document['webkitFullscreenElement'] ||
        document['webkitCurrentFullScreenElement']) === canvasContainer) {
        canvas.exitFullscreen = document['exitFullscreen'] ||
          document['cancelFullScreen'] ||
          document['mozCancelFullScreen'] ||
          document['msExitFullscreen'] ||
          document['webkitCancelFullScreen'] ||
          function () { };
        canvas.exitFullscreen = canvas.exitFullscreen.bind(document);
        if (Browser.lockPointer) canvas.requestPointerLock();
        Browser.isFullscreen = true;
        if (Browser.resizeCanvas) Browser.setFullscreenCanvasSize();
      } else {
        if (Browser.resizeCanvas) Browser.setWindowedCanvasSize();
      }
      if (Module['onFullScreen']) Module['onFullScreen'](Browser.isFullscreen);
      if (Module['onFullscreen']) Module['onFullscreen'](Browser.isFullscreen);
      Browser.updateCanvasDimensions(canvas);
    }

    if (!Browser.fullscreenHandlersInstalled) {
      Browser.fullscreenHandlersInstalled = true;
      document.addEventListener('fullscreenchange', fullscreenChange, false);
      document.addEventListener('mozfullscreenchange', fullscreenChange, false);
      document.addEventListener('webkitfullscreenchange', fullscreenChange, false);
      document.addEventListener('MSFullscreenChange', fullscreenChange, false);
    }

    // create a new parent to ensure the canvas has no siblings. this allows browsers to optimize full screen performance when its parent is the full screen root
    var canvasContainer = canvas.parentNode;
    /*document.createElement("div");
    canvas.parentNode.insertBefore(canvasContainer, canvas);
    canvasContainer.appendChild(canvas);*/

    // use parent of canvas as full screen root to allow aspect ratio correction (Firefox stretches the root to screen size)
    canvasContainer.requestFullscreen = canvasContainer['requestFullscreen'] ||
      canvasContainer['mozRequestFullScreen'] ||
      canvasContainer['msRequestFullscreen'] ||
      (canvasContainer['webkitRequestFullscreen'] ? function () { canvasContainer['webkitRequestFullscreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null) ||
      (canvasContainer['webkitRequestFullScreen'] ? function () { canvasContainer['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null);

    canvasContainer.requestFullscreen();
  },
  setStatus: function (text) {
    if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
    if (text === Module.setStatus.text) return;
    var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
    var now = Date.now();
    document.getElementById('status').innerHTML = text;
    cout_print("setStatus: "+text);
  },
  totalDependencies: 0,
  monitorRunDependencies: function (left) {
    this.totalDependencies = Math.max(this.totalDependencies, left);
    Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies - left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
  }
};

window.onerror = function (event) {
  // TODO: do not warn on ok events like simulating an infinite loop or exitStatus
  Module.setStatus('Exception thrown, '+String(event));
  Module.setStatus = function (text) {
    if (text) Module.printErr('[post-exception status] ' + text);
  };
};

var img_data32;
function my_SDL_LockSurface(surf) {
  var surfData = SDL.surfaces[surf];
  surfData.locked++;
  if (surfData.locked > 1) return 0;

  if (!surfData.buffer) {
    surfData.buffer = _malloc(surfData.width * surfData.height * 4);
    cout_print("malloc " + surfData.buffer + "\n");
    HEAP32[(((surf) + (20)) >> 2)] = surfData.buffer;
  }

  // Mark in C/C++-accessible SDL structure
  // SDL_Surface has the following fields: Uint32 flags, SDL_PixelFormat *format; int w, h; Uint16 pitch; void *pixels; ...
  // So we have fields all of the same size, and 5 of them before us.
  // TODO: Use macros like in library.js
  HEAP32[(((surf) + (20)) >> 2)] = surfData.buffer;
  if (!surfData.image) {
    surfData.image = surfData.ctx.getImageData(0, 0, surfData.width, surfData.height);
  }
  return 0;
}
function my_SDL_UnlockSurface(surf) {
  assert(!SDL.GL); // in GL mode we do not keep around 2D canvases and contexts

  var surfData = SDL.surfaces[surf];

  if (!surfData.locked || --surfData.locked > 0) {
    return;
  }
  var data = surfData.image.data;
  var src = surfData.buffer >> 2;
  // Copy pixel data to image
  if (!img_data32)
  { img_data32 = new Uint32Array(data.buffer); }
  img_data32.set(HEAP32.subarray(src, src + img_data32.length));

  surfData.ctx.putImageData(surfData.image, 0, 0);
  // Note that we save the image, so future writes are fast. But, memory is not yet released
}


var padStatus1;
var padStatus2;
var vram_ptr;
var cout_print = Module.print;
var pcsx_worker;
var SoundFeedStreamData;

function var_setup() {
  SoundFeedStreamData = Module.cwrap("SoundFeedStreamData", "null", ["number", "number"]);
  vram_ptr = _get_ptr(0);
  padStatus1 = _get_ptr(1);
  padStatus2 = _get_ptr(2);  
  SDL.defaults.copyOnLock = false;
  SDL.defaults.opaqueFrontBuffer = false;
  cout_print("start worker")
  pcsx_worker = new Worker("pcsx_worker.js");
  pcsx_worker.onmessage = pcsx_worker_onmessage;
  document.getElementById('iso_opener').disabled=false;
  var spinner = document.getElementById('spinner');
  spinner.parentElement.removeChild(spinner);  
  setTimeout("Module.setStatus('open an iso file using the above button.')", 2);
}

if (window.File && window.FileReader && window.FileList && window.Blob) {
} else {
  alert('The File APIs are not fully supported in this browser.')
  cout_print('The File APIs are not fully supported in this browser.');
}
/* // for future wakelock api
navigator.wakeLock.request("display")
  .then(() => cout_print("Display wakeLock OK\n"))
  .catch(() => cout_print("Display wakeLock failed\n"));
  
navigator.wakeLock.request("system")
  .then(() => cout_print("System wakeLock OK\n"))
  .catch(() => cout_print("System wakeLock failed\n"));
*/
var states_arrs = [];
var check_controller = function () {
  _CheckJoy();
  _CheckKeyboard();
  var states_src = Module.HEAPU8.subarray(padStatus1, padStatus1 + 48);
  var states_arr;
  while (states_arrs.length > 50) {
    states_arrs.pop();
  }
  if (states_arrs.length > 0) {
    states_arr = states_arrs.pop();
    states_arr.set(states_src);
  }
  else {
    states_arr = new Uint8Array(states_src);
  }
  //if(stat!=65535)  cout_print(stat);
  pcsx_worker.postMessage({ cmd: "padStatus", states: states_arr }, [states_arr.buffer]);
  setTimeout("check_controller()", 10);
}

var file_list;
var pcsx_readfile = function (controller) {
  document.getElementById('iso_opener').disabled=true;  
  cout_print("pcsx_readfile\n");
  file_list = controller.files;
  pcsx_worker.postMessage({ cmd: "loadfile", file: controller.files[0] });
  setTimeout("check_controller()", 10);
  return;
}

function pcsx_worker_onmessage(event) {
  var data = event.data
  // cout_print("onmessage: "+data.cmd)
  switch (data.cmd) {
    case "print":
      cout_print("> " + data.txt);
      break
    case "setStatus":
      cout_print("cmd setStatus")
      Module.setStatus(data.txt);
      break
    case "setUI":
      var el = document.getElementById(data.key)      
      for (var k in data.properties) {
          el[k] = data.properties[k];
        }
      break
    case "render":
      var vram_arr = data.vram;
      Module.HEAPU8.set(vram_arr, vram_ptr);
      pcsx_worker.postMessage({ cmd: "return_vram", vram: vram_arr }, [vram_arr.buffer]);
      _render(data.x, data.y, data.sx, data.sy, data.dx, data.dy, data.rgb24);
      break
    case "return_states":
      states_arrs.push(data.states)
      break;
    case "SoundFeedStreamData":
      var pSound_arr = data.pSound;
      var pSound_ptr = Module._malloc(pSound_arr.length);
      Module.HEAPU8.set(pSound_arr, pSound_ptr);
      SoundFeedStreamData(pSound_ptr, data.lBytes);
      Module._free(pSound_ptr);
      break
    default:
      cout_print("unknown worker cmd " + data.cmd)
  }
}



