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
 var x_ptr, y_ptr, dx_ptr, dy_ptr, rgb24_ptr, vram_ptr;
var vram_arrs=[];
 var render = function()  {
	 var vram_arr;
	 var vram_src = Module.HEAPU8.subarray(vram_ptr, vram_ptr+480*2048);
	 if(vram_arrs.length>0){
		 vram_arr=vram_arrs.pop();
		 vram_arr.set(vram_src);
		}
	 else{
		cout_print("create vram\n");
	 	vram_arr = new Uint8Array(vram_src);
	 }
	 postMessage({cmd: "render", 
	 		x:getValue(x_ptr, 'i32'),
		y:getValue(y_ptr, 'i32'),
		 		dx:getValue(dx_ptr, 'i16'),
		dy:getValue(dy_ptr, 'i32'),
		rgb24: getValue(rgb24_ptr, 'i32'),
		vram: vram_arr
}, [vram_arr.buffer]);
 } 


 function pcsx_mainloop(){
	 _one_iter()
	 render();
	 setTimeout("pcsx_mainloop()", 0);	      
}
onmessage=function(event)  {  
  var data=event.data;
	//cout_print("worker onmessage\n");
  switch(data.cmd){
		case "padStatus":
      _set_KeyStatus(0, data.KeyStatus0);
    break;
		case "return_vram":
			vram_arrs.push(data.vram)
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
		  cout_print("_.bin loaded")		  
		  Module.FS_createDataFile("/", "_.bin", new Uint8Array(this.result) , true, true);
      Module.setStatus('Running...');
      Module._pcsx_init();
			x_ptr = _get_render_param_ptr(0);
			y_ptr = _get_render_param_ptr(1);
			dx_ptr = _get_render_param_ptr(2);
			dy_ptr = _get_render_param_ptr(3);
			rgb24_ptr = _get_render_param_ptr(4);
			vram_ptr = _get_render_param_ptr(5);
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