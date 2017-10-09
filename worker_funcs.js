var Module;
if (!Module) Module = {};
Module.setStatus = function (s) {
	postMessage({
		cmd: "print",
		txt: s
	});
};

function cout_print(s) {
	postMessage({
		cmd: "print",
		txt: s
	});
}

function set_progress(k, r) {
	postMessage({
		cmd: "setUI",
		key: k + "_progress",
		properties: r
	});
}


function show_error(prefix) {
	return function (evt) {
		cout_print(prefix);
		cout_print(String(evt.target.error));
	}
}

Module['print'] = cout_print;
var vram_ptr, soundbuffer_ptr, isMute_ptr;
var vram_dels = 0,
	vram_cres = 0;
var vram_arrs = [];
var render = function (x, y, sx, sy, dx, dy, rgb24) {
	var vram_arr;
	var vram_src = Module.HEAPU8.subarray(vram_ptr, vram_ptr + 1024 * 2048);
	while (vram_arrs.length > 10) {
		vram_arrs.pop();
		vram_dels++;
		//cout_print("delete vram "+vram_dels+"\n");
	}
	if (vram_arrs.length > 0) {
		vram_arr = vram_arrs.pop();
		vram_arr.set(vram_src);
	} else {
		vram_cres++;
		//cout_print("create vram "+vram_cres+"\n");
		vram_arr = new Uint8Array(vram_src);
	}
	postMessage({
		cmd: "render",
		x: x,
		y: y,
		sx: sx,
		sy: sy,
		dx: dx,
		dy: dy,
		rgb24: rgb24,
		vram: vram_arr
	}, [vram_arr.buffer]);
}
var pSound_arrs = [];
var SendSound = function (pSound_ptr, lBytes) {
	var pSound_arr;
	var pSound_src = Module.HEAPU8.subarray(pSound_ptr, pSound_ptr + lBytes);
	while (pSound_arrs.length > 50) {
		pSound_arrs.pop();
	}
	if (pSound_arrs.length > 0) {
		pSound_arr = pSound_arrs.pop();
	} else {
		pSound_arr = new Uint8Array(4800);
	}

	pSound_arr.set(pSound_src);
	postMessage({
		cmd: "SoundFeedStreamData",
		pSound: pSound_arr,
		lBytes: lBytes
	}, [pSound_arr.buffer]);
}


function pcsx_mainloop() {
	_one_iter();

}
var pcsx_init = Module.cwrap("pcsx_init", "number", ["string"])
var ls = Module.cwrap("ls", "null", ["string"])
var padStatus1;
var isoDB;
var stdout_array;
var readfile_and_run = function (iso_name, blob) {
	var run_arr = function (arr) {
		//Module.FS_createDataFile("/", iso_name, arr, true, true);
		stdout_array = arr;
		Module.setStatus('Running!');
		pcsx_init("/" + iso_name);
		padStatus1 = _get_ptr(-2);
		vram_ptr = _get_ptr(-1);
		soundbuffer_ptr = _get_ptr(7);
		isMute_ptr = _get_ptr(8);
		cout_print("before mainloop\n");
		pcsx_mainloop();
	}
	cout_print("readfile and run ");
	var reader = new FileReader();
	Module.setStatus("reading file");
	reader.onprogress = function (e) {
		if (e.lengthComputable) {
			//cout_print(Math.round((e.loaded / e.total) * 100) + "%");
			set_progress('readfile', {
				value: e.loaded,
				max: e.total,
				hidden: false
			});
		} else
			cout_print(e.loaded + "bytes")
		//document.getElementById("start").disabled=false		
	}
	reader.onload = function (e) {
		cout_print("" + iso_name + " loaded");
		set_progress('readfile', {
			value: 1,
			max: 1,
			hidden: false
		});
		run_arr(new Uint8Array(this.result))
	}
	reader.readAsArrayBuffer(blob);
}

var event_history = [];
var clear_event_history = function () {
	self.onmessage = main_onmessage;
	for (var i in event_history) {
		main_onmessage(event_history[i]);
	}
	event_history = [];
	Module.setStatus = function (s) {
		postMessage({
			cmd: "setStatus",
			txt: s
		});
	};
	setTimeout("Module.setStatus('Open an iso file using the above button(worker ready!).')", 1);
}
var pre_onmessage = function (event) {
	if (event.data.cmd != 'soundBytes') {
		event_history.push(event);
		cout_print("push event" + event.data.cmd);
	}
}
self.onmessage = pre_onmessage;
var main_onmessage = function (event) {
	var data = event.data;
	switch (data.cmd) {

		case "padStatus":
			Module.HEAPU8.set(data.states, padStatus1);
			postMessage({
				cmd: "return_states",
				states: data.states
			}, [data.states.buffer]);
			//Module.setValue(soundbuffer_ptr, data.soundbuffer, "i32");
			break;

		case "soundBytes":
			Module.setValue(soundbuffer_ptr, Module.getValue(soundbuffer_ptr, "i32") - data.lBytes, "i32");
			break;

		case "return_vram":
			vram_arrs.push(data.vram)
			break;

		case "return_pSound":
			pSound_arrs.push(data.pSound)
			break;

		case "ls":
			ls(data.dir);
			break;
		case "loadfile":
			Module.setStatus('Downloading...');
			cout_print(data.file.name)
			readfile_and_run(data.file.name, data.file);
			break;

		case "loadurl":
			cout_print("load..." + data.iso);

			load_or_fetch(data.iso)

			break;

		default:
			postMessage({
				cmd: "print",
				txt: "unknown command " + data.cmd
			})
	}
}
cout_print("worker started\n");
onerror = function (event) {
	// TODO: do not warn on ok events like simulating an infinite loop or exitStatus	
	Module.setStatus('Exception thrown, see JavaScript console ' + String(event));
};