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


 Module['print'] = cout_print;
 var x_ptr, y_ptr, sx_ptr, sy_ptr, dx_ptr, dy_ptr, rgb24_ptr, vram_ptr,soundbuffer_ptr;
 var vram_dels=0, vram_cres=0;
var vram_arrs=[];
 var render = function(x,y,sx,sy,dx,dy,rgb24)  {
	 var vram_arr;
	 var vram_src = Module.HEAPU8.subarray(vram_ptr, vram_ptr+1024*2048);
	 while(vram_arrs.length>10){
		 vram_arrs.pop();
		 vram_dels++;
		 //cout_print("delete vram "+vram_dels+"\n");
	 }
	 if(vram_arrs.length>0){
		 vram_arr=vram_arrs.pop();
		 vram_arr.set(vram_src);
		}
	 else{
		vram_cres++;
		//cout_print("create vram "+vram_cres+"\n");
	 	vram_arr = new Uint8Array(vram_src);
	 }
	 postMessage({cmd: "render", x:x,y:y,sx:sx,sy:sy,dx:dx,dy:dy,rgb24:rgb24,vram: vram_arr}, [vram_arr.buffer]);
 } 
var pSound_arrs=[];
 var SendSound = function(pSound_ptr, lBytes) {
   var pSound_arr;
	 var pSound_src = Module.HEAPU8.subarray(pSound_ptr, pSound_ptr+lBytes);
	 while(pSound_arrs.length>30){
		 pSound_arrs.pop();
	 }
	 if(pSound_arrs.length>0){
		 pSound_arr=pSound_arrs.pop();
		}
	 else{
	 	pSound_arr = new Uint8Array(4096);
	 }

	  pSound_arr.set(pSound_src);		 
		 postMessage({cmd: "SoundFeedStreamData", 
		 		pSound: pSound_arr, lBytes: lBytes
			}, [pSound_arr.buffer]);
 }


 function pcsx_mainloop(){
	 _one_iter();
	      
}
var pcsx_init = Module.cwrap("pcsx_init", "number", ["string"])
var ls = Module.cwrap("ls", "null", ["string"])
var padStatus1; 
onmessage=function(event)  {  
  var data=event.data;
	//cout_print("worker onmessage\n");
  switch(data.cmd){

		case "padStatus":
      Module.HEAPU8.set(data.states, padStatus1);
			postMessage({cmd: "return_states", states: data.states}, [data.states.buffer]); 
			//Module.setValue(soundbuffer_ptr, data.soundbuffer, "i32");
    break;
		case "soundBytes":
			Module.setValue(soundbuffer_ptr, Module.getValue(soundbuffer_ptr, "i32")-data.lBytes, "i32");
		break
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
		  cout_print(""+f.name+" loaded")		  
		  Module.FS_createDataFile("/", f.name, new Uint8Array(this.result) , true, true);
      Module.setStatus('Running...');
      pcsx_init("/"+f.name);
			padStatus1 =  _get_ptr(-2);
			x_ptr = _get_ptr(0);
			y_ptr = _get_ptr(1);
			sx_ptr = _get_ptr(2);
			sy_ptr = _get_ptr(3);
			rgb24_ptr = _get_ptr(4);
			dx_ptr = _get_ptr(5);
			dy_ptr = _get_ptr(6);
			vram_ptr = _get_ptr(-1);
			soundbuffer_ptr = _get_ptr(7);
			cout_print("before mainloop\n");
			pcsx_mainloop();
		}	      
       reader.readAsArrayBuffer(f);
	break

    case "frameskip":
      frameskip=data.value
      break
    default:
      postMessage({cmd:"print", txt:"unknown command "+data.cmd})
  }
}
cout_print("worker started\n");