static const char main_page[] = R"=====(
<!DOCTYPE html>
<html lang="en" style="background-color: #004">
<head>
	<meta charset="UTF-8">
	<title>nikthefix-LVGL-web</title>
</head>

<script language="javascript" type="text/javascript">

var canvas;
var context;
var imageData;
var width;
var height;

var websocket;
var ws_connected;

var pointerDown;
var canvas_left;
var canvas_top;

function init() {
	canvas = document.getElementById("canvas");
	context = canvas.getContext("2d")
	width = canvas.width;
	height = canvas.height;
	pointerDown = false;
	
	if (window.PointerEvent) {
		canvas.addEventListener('pointerdown', onPointerDown);
		canvas.addEventListener('pointermove', onPointerMove);
		canvas.addEventListener('pointerup', onPointerUp);
		canvas.addEventListener('pointerleave', onPointerUp);
		canvas.addEventListener('pointercancel', onPointerUp);
	} else {
		canvas.addEventListener('mousedown', onPointerDown);
		canvas.addEventListener('mousemove', onPointerMove);
		canvas.addEventListener('mouseup', onPointerUp);
		canvas.addEventListener('mouseleave', onPointerUp);
	}

	ws_connected = false;
	wsConnect();
}

function wsConnect() {
	websocket = new WebSocket('ws://'+location.hostname+'/');
	websocket.binaryType = "arraybuffer";
	websocket.onopen = function(evt) { onOpen(evt) };
	websocket.onclose = function(evt) { onClose(evt) };
	websocket.onmessage = function(evt) { onMessage(evt) };
	websocket.onerror = function(evt) { onError(evt) };
}

function wsSend(state, x, y) {
	if (ws_connected) {
		var mouse_packet = new Uint8Array(5);
		
		mouse_packet[0] = state;
		mouse_packet[1] = (x >> 8) & 0xFF;
		mouse_packet[2] = x & 0xFF;
		mouse_packet[3] = (y >> 8) & 0xFF;
		mouse_packet[4] = y & 0xFF;
		
		websocket.send(mouse_packet);
	}
}

function onOpen(evt) {
	console.log("Connected");
	ws_connected = true;
}

function onClose(evt) {
	console.log("Disconnected");
	ws_connected = false;
	setTimeout(function() { wsConnect() }, 2000);
}

function onMessage(evt) {
	var buffer = evt.data;
	var header = new Uint8Array(buffer, 0, 13);
	var pixels = new Uint8Array(buffer, 13);
	var pixel_depth = header[0];
	var w  = (header[1] << 8) | header[2];
	var h  = (header[3] << 8) | header[4];
	var x1 = (header[5] << 8) | header[6];
	var y1 = (header[7] << 8) | header[8];
	var x2 = (header[9] << 8) | header[10];
	var y2 = (header[11] << 8) | header[12];
	var pixelIndex = 0;
	
	if ((w != width) || (h != height)) {
		canvas.width = w;
		canvas.height = h;
		width = w;
		height = h;
		
		context.fillStyle = "black";
		context.fillRect(0, 0, width, height);
		imageData = context.getImageData(0, 0, width, height);
	}

	if (pixel_depth == 32) {
		for (var y=y1; y<=y2; y++) {
			for (var x=x1; x<=x2; x++) {
				var canvasIndex = (y * width + x) * 4;
				imageData.data[canvasIndex    ] = pixels[pixelIndex++];
				imageData.data[canvasIndex + 1] = pixels[pixelIndex++];
				imageData.data[canvasIndex + 2] = pixels[pixelIndex++];
				imageData.data[canvasIndex + 3] = pixels[pixelIndex++];
			}
		}
	} else if (pixel_depth == 16) {
		for (var y=y1; y<=y2; y++) {
			for (var x=x1; x<=x2; x++) {
				var canvasIndex = (y * width + x) * 4;
				var c16 = (pixels[pixelIndex++] << 8) | pixels[pixelIndex++];
				imageData.data[canvasIndex    ] = (c16 & 0xF800) >> 8;
				imageData.data[canvasIndex + 1] = (c16 & 0x07E0) >> 3;
				imageData.data[canvasIndex + 2] = (c16 & 0x001F) << 3;
				imageData.data[canvasIndex + 3] = 255;
			}
		}
	} else {
		for (var y=y1; y<=y2; y++) {
			for (var x=x1; x<=x2; x++) {
				var canvasIndex = (y * width + x) * 4;
				var c8 = pixels[pixelIndex++];
				imageData.data[canvasIndex    ] = (c8 & 0xE0);
				imageData.data[canvasIndex + 1] = (c8 & 0x1C) << 3;
				imageData.data[canvasIndex + 2] = (c8 & 0x03) << 6;
				imageData.data[canvasIndex + 3] = 255;
			}
		}
	}
	context.putImageData(imageData, 0, 0);
}

function onError(evt) {
    console.log("ERROR: " + evt);
}

function onPointerDown(evt) {
	var rect = canvas.getBoundingClientRect();
	pointerDown = true;
	var x = (evt.clientX - rect.left) / (rect.right - rect.left) * canvas.width;
	var y = (evt.clientY - rect.top) / (rect.bottom - rect.top) * canvas.height;
	wsSend(1, x, y);
	console.log("Canvas X:", x, "Canvas Y:", y);
}

function onPointerMove(evt) {
	var rect = canvas.getBoundingClientRect();
	if (pointerDown) {
		evt.preventDefault();
		var x = (evt.clientX - rect.left) / (rect.right - rect.left) * canvas.width;
	  var y = (evt.clientY - rect.top) / (rect.bottom - rect.top) * canvas.height;
		wsSend(1, x, y);
		console.log("Canvas X:", x, "Canvas Y:", y);
	}
}

function onPointerUp(evt) {
	var rect = canvas.getBoundingClientRect();
	pointerDown = false;
	var x = (evt.clientX - rect.left) / (rect.right - rect.left) * canvas.width;
	var y = (evt.clientY - rect.top) / (rect.bottom - rect.top) * canvas.height;
	wsSend(0, x, y);
	console.log("Canvas X:", x, "Canvas Y:", y);
}

window.addEventListener("load", init, false);
</script>

<body>
        <div id='d1' style="position:absolute; top:100px; left:100px; z-index:1">
	<canvas id="canvas" width="1" height="1"></canvas>
</body>
</html>

)=====";
