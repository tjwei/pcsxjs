var Module;
if (!Module) Module = {};
Module.setStatus = function(s){
	postMessage({cmd:"setStatus", txt:s});
}

function cout_print(s){
	postMessage({cmd:"print", txt:s});
}
function set_progress(r){
	postMessage({cmd:"setProgress", properties:r});
}
function snes_init(){
      reboot_emulator=Module.cwrap('reboot_emulator', 'number', ['string'])
	   native_set_joypad_state=Module._native_set_joypad_state
	   native_bitmap_pointer=Module._native_bitmap_pointer
	   mainloop=Module._mainloop
	   renderscreen=Module._renderscreen
	   palf=reboot_emulator("/_.smc")	
	   native_set_joypad_state(0x80000000)
	   frameskip=0

           }
function snes_mainloop(){		
		for(var _i=0;_i<=frameskip;_i++)
		  mainloop(palf ? 312 :262)		
		renderscreen()		  
		var bitmap=native_bitmap_pointer(-16,0)>>2		
		var src=i.subarray(bitmap,bitmap+288*224)  // // Unstable Hack: i is Heap32 compiled by closure
		var buffer=new ArrayBuffer(4*288*224)
		var src2=new Uint32Array(buffer)			// WW2
		for(var _i=0;_i<288*224;_i++) src2[_i]=src[_i]		// WW2
		postMessage({cmd:"render2", src:buffer}, [buffer])    // WW2
		postMessage({cmd:"render", src:src})			// WW1
		setTimeout("snes_mainloop()", 0);	      
}
 Module['print'] = cout_print;

 var render = function(){
	 postMessage({cmd: "render"});
 }
onmessage=function(event) {  
  var data=event.data;
	//cout_print("worker onmessage\n");
  switch(data.cmd){
    case "loadfile": 
    		Module.setStatus('Downloading...');
	      var f=data.file;
	      cout_print(f.name)
	      var reader= new FileReader();
	      reader.onprogress = function(e){
		if (e.lengthComputable){
		    cout_print(Math.round((e.loaded / e.total) * 100)+"%");
				set_progress({value: e.loaded, max: e.total, hidden: false});
		}
		else
		  count_print(e.loaded+"bytes")
		   //document.getElementById("start").disabled=false		
	      }
	    reader.onload = function(e) {		 
		  cout_print("_.bin loaded")		  
		  Module.FS_createDataFile("/", "_.bin", new Uint8Array(this.result) , true, true);
      Module.setStatus('Running...');
      Module._pcsx_init();
			postMessage({cmd:"emulatorReady"});
		}	      
       reader.readAsArrayBuffer(f);
	break
    case "one_iter":
      _one_iter()
			render();
      break
    case "joy1":
      native_set_joypad_state(data.state)
      break
    case "frameskip":
      frameskip=data.value
      break
    default:
      postMessage({cmd:"print", txt:"unknown command "+data.cmd})
  }
}
postMessage({cmd:"print", txt:"worker started\n"})