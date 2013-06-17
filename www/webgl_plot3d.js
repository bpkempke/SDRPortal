var gl;
var canvas;
var shaderProgram;
var mesh_vertex_buffer;
var mesh_color_buffer;
var mesh_element_buffer;
var mesh_width = 0;
var mesh_length = 0;
var current_row = 0;
var current_swap_row = 0;
var mesh_indices_temp;
var fpsCounter;

var mvMatrix = mat4.create();
var mvMatrixStack = [];
var pMatrix = mat4.create();

function createGLContext(canvas) {
  var names = ["webgl", "experimental-webgl"];
  var context = null;
  for (var i=0; i < names.length; i++) {
    try {
      context = canvas.getContext(names[i]);
    } catch(e) {}
    if (context) {
      break;
    }
  }
  if (context) {
    context.viewportWidth = canvas.offsetWidth;
    context.viewportHeight = canvas.offsetHeight;
  } else {
    alert("Failed to create WebGL context!");
  }
  return context;
}

function loadShaderFromDOM(id) {
  var shaderScript = document.getElementById(id);
  
  // If we don't find an element with the specified id
  // we do an early exit 
  if (!shaderScript) {
    return null;
  }
  
  // Loop through the children for the found DOM element and
  // build up the shader source code as a string
  var shaderSource = "";
  var currentChild = shaderScript.firstChild;
  while (currentChild) {
    if (currentChild.nodeType == 3) { // 3 corresponds to TEXT_NODE
      shaderSource += currentChild.textContent;
    }
    currentChild = currentChild.nextSibling;
  }
 
  var shader;
  if (shaderScript.type == "x-shader/x-fragment") {
    shader = gl.createShader(gl.FRAGMENT_SHADER);
  } else if (shaderScript.type == "x-shader/x-vertex") {
    shader = gl.createShader(gl.VERTEX_SHADER);
  } else {
    return null;
  }
 
  gl.shaderSource(shader, shaderSource);
  gl.compileShader(shader);
 
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    alert(gl.getShaderInfoLog(shader));
    return null;
  } 
  return shader;
}

function setupShaders() {
  vertexShader = loadShaderFromDOM("shader-vs");
  fragmentShader = loadShaderFromDOM("shader-fs");
  
  shaderProgram = gl.createProgram();
  gl.attachShader(shaderProgram, vertexShader);
  gl.attachShader(shaderProgram, fragmentShader);
  gl.linkProgram(shaderProgram);

  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
    alert("Failed to setup shaders");
  }

  gl.useProgram(shaderProgram);
  
  shaderProgram.vertexPositionAttribute = gl.getAttribLocation(shaderProgram, "aVertexPosition"); 
  gl.enableVertexAttribArray(shaderProgram.vertexPositionAttribute);

  shaderProgram.vertexColorAttribute = gl.getAttribLocation(shaderProgram, "aVertexColor");
  gl.enableVertexAttribArray(shaderProgram.vertexColorAttribute);

  shaderProgram.pMatrixUniform = gl.getUniformLocation(shaderProgram, "uPMatrix");
  shaderProgram.mvMatrixUniform = gl.getUniformLocation(shaderProgram, "uMVMatrix");
  shaderProgram.offsetUniform = gl.getUniformLocation(shaderProgram, "uOffset");
  shaderProgram.samplerUniform = gl.getUniformLocation(shaderProgram, "uSampler");
}

function setupBuffers(width, length) {
  mesh_width = width;
  mesh_length = length;
  current_swap_row = length-1;
  
  mesh_vertex_buffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, mesh_vertex_buffer);
  var mesh_vertices = new Array(width*length*2);
  for(var ii=0; ii < length; ii=ii+1){
    for(var jj=0; jj < width; jj=jj+1){
      mesh_vertices[(ii*width+jj)*2] = jj/width-0.5;//X Coordinate
      mesh_vertices[(ii*width+jj)*2+1] = ii/width-0.5;//Y Coordinate
    }
  }
  
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(mesh_vertices), gl.STATIC_DRAW);
  mesh_vertex_buffer.itemSize = 2;
  mesh_vertex_buffer.numberOfItems = mesh_vertices.length/2;

  mesh_element_buffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, mesh_element_buffer);
  var width_ext = width*2 + 2;
  var mesh_indices = new Array(width_ext*(length-1));
  for(var ii=0; ii < length-1; ii=ii+1){
    mesh_indices[current_swap_row*width_ext] = ii*width;
    for(var jj=0; jj < width; jj=jj+1){
      mesh_indices[current_swap_row*width_ext+jj*2+1] = ii*width+jj;
      mesh_indices[current_swap_row*width_ext+jj*2+2] = (ii+1)*width+jj;
    }
    mesh_indices[(current_swap_row+1)*width_ext-1] = (ii+2)*width-1;
    current_swap_row = current_swap_row - 1;
  }
  mesh_indices_temp = mesh_indices;
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(mesh_indices), gl.DYNAMIC_DRAW);
  mesh_element_buffer.itemSize = 1;
  mesh_element_buffer.numberOfItems = width_ext*(length-1);
  
  mesh_color_buffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, mesh_color_buffer);
  var mesh_colors = new Array(width*length);
  //Fill in some temporary color data for the mesh for now, then successive adds will change this
  for(var ii=0; ii < length; ii=ii+1){
    for(var jj=0; jj < width; jj=jj+1){
      mesh_colors[ii*width+jj] = 0;//Math.random();
    }
  }
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(mesh_colors), gl.DYNAMIC_DRAW);
  mesh_color_buffer.itemSize = 1;
  mesh_color_buffer.numberOfItems = mesh_colors.length;
}

function handleLoadedTexture(texture) {
  gl.bindTexture(gl.TEXTURE_2D, texture);
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texture.image);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.bindTexture(gl.TEXTURE_2D, null);
 } 

var paletteTexture;
function initPalette(){
  paletteTexture = gl.createTexture();
  paletteTexture.image = new Image();
  paletteTexture.image.onload = function(){
    handleLoadedTexture(paletteTexture)
  }
  paletteTexture.image.src = "jet.bmp";
}

function setMatrixUniforms() {
  gl.uniformMatrix4fv(shaderProgram.pMatrixUniform, false, pMatrix);
  gl.uniformMatrix4fv(shaderProgram.mvMatrixUniform, false, mvMatrix);
  gl.uniform1f(shaderProgram.offsetUniform, current_row/mesh_length);
}

function shiftInRow(data){
  //Add the data into the current 'color' array
  gl.bindBuffer(gl.ARRAY_BUFFER, mesh_color_buffer);
  gl.bufferSubData(gl.ARRAY_BUFFER, current_row*mesh_width*4, new Float32Array(data));

  //Fix garbling because of tying end into beginning
  var width_ext = mesh_width*2 + 2;
  var mesh_indices = new Array(width_ext);
  var current_m1 = (current_row == 0) ? mesh_length-1 : (current_row-1);
  mesh_indices[0] = current_m1*mesh_width;
  for(var jj=0; jj < mesh_width; jj=jj+1){
    mesh_indices[jj*2+1] = current_m1*mesh_width+jj;
    mesh_indices[jj*2+2] = current_row*mesh_width+jj;
  }
  mesh_indices[width_ext-1] = (current_row+1)*mesh_width-1;
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, mesh_element_buffer);
  gl.bufferSubData(gl.ELEMENT_ARRAY_BUFFER, current_swap_row*width_ext*2, new Uint16Array(mesh_indices));

  //Increment current row in matrix
  current_row = current_row + 1;
  if(current_row >= mesh_length) current_row = 0;
  current_swap_row = current_swap_row - 1;
  if(current_swap_row < 0) current_swap_row = mesh_length - 2;
}

var currentTime = 0;
var nbrOfFramesForFPS = 0;
var previousFrameTimeStamp = 0;
function webgl_plot3d_draw() { 
  currentTime = Date.now();

   // Update FPS if a second or more has passed since last FPS update
  if(currentTime - previousFrameTimeStamp >= 1000) {
    fpsCounter.innerHTML = nbrOfFramesForFPS;
    nbrOfFramesForFPS = 0;
    previousFrameTimeStamp = currentTime;                    
  } 
 
  gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
  gl.clear(gl.COLOR_BUFFER_BIT);
  
  mat4.perspective(48, gl.viewportWidth / gl.viewportHeight, 0.1, 100.0, pMatrix);
  mat4.identity(mvMatrix);
  mat4.lookAt([-0.75,-0.75,0.75],[-0.15,-0.15,0],[1,1,0], mvMatrix);
  mat4.translate(mvMatrix,[0,-1,0],mvMatrix);
  

  //Bind the color palette texture so that we get some colors in the scene
  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, paletteTexture);
  gl.uniform1i(shaderProgram.samplerUniform, 0);

  // Draw the independent triangle
  // For the triangle we want to use per-vertex color so
  // we enable the vertexColorAttribute again
  gl.enableVertexAttribArray(shaderProgram.vertexColorAttribute);
  
  gl.bindBuffer(gl.ARRAY_BUFFER, mesh_vertex_buffer);
  gl.vertexAttribPointer(shaderProgram.vertexPositionAttribute, 
                         mesh_vertex_buffer.itemSize, gl.FLOAT, false, 0, 0);
                         
  gl.bindBuffer(gl.ARRAY_BUFFER, mesh_color_buffer);  
  gl.vertexAttribPointer(shaderProgram.vertexColorAttribute, 
                         mesh_color_buffer.itemSize, gl.FLOAT, false, 0, 0);
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, mesh_element_buffer);

  setMatrixUniforms();
  gl.drawElements(gl.TRIANGLE_STRIP, mesh_element_buffer.numberOfItems, gl.UNSIGNED_SHORT, 0);
                                                 
  nbrOfFramesForFPS++;
}

var canvas2d_width, canvas2d_height;
var canvas2d_context;
var canvas2d_palette;
var canvas2d_buffer;
var canvas2d_final_context;
var canvas2d_final_width;
var canvas2d_final_height;
function create2DContext(canvas,width,height){
  canvas2d_width = width;
  canvas2d_height = height;
  canvas2d_buffer = document.createElement('canvas');
  canvas2d_buffer.width = width;
  canvas2d_buffer.height = height;
  canvas2d_final_context = canvas.getContext("2d");
  canvas2d_final_width = canvas.width;
  canvas2d_final_height = canvas.height;
  return canvas2d_buffer.getContext("2d");
}

function fountain2DClear(){
  //First extract out the color map
  canvas2d_context.drawImage(paletteTexture.image,0,0);
  canvas2d_palette = canvas2d_context.getImageData(0,0,1,64).data;

  //Now clear everything out
  canvas2d_context.fillStyle = "black";
  canvas2d_context.fillRect(0,0,canvas2d_width,canvas2d_height);
}

function fountain2DShift(data){

  //Shift current image up by one pixel
  var current_image = canvas2d_context.getImageData(0,0,canvas2d_width,canvas2d_height);
  canvas2d_context.putImageData(current_image,0,-1);

  //Make a new image for the last row of pixels
  var new_row = canvas2d_context.createImageData(canvas2d_width, 1);
  for(var ii = 0; ii < canvas2d_width; ii++){
    var color_index = Math.floor(data[ii]*64)*4;
    new_row.data[ii*4] = canvas2d_palette[color_index];
    new_row.data[ii*4+1] = canvas2d_palette[color_index+1];
    new_row.data[ii*4+2] = canvas2d_palette[color_index+2];
    new_row.data[ii*4+3] = canvas2d_palette[color_index+3];
  }
  canvas2d_context.putImageData(new_row,0,canvas2d_height-1);

  //Now copy over to the stretched canvas
  canvas2d_final_context.drawImage(canvas2d_buffer, 0, 0, canvas2d_final_width, canvas2d_final_height);
}

var canvas2dplot_length;
var canvas2dplot_context;
var canvas2dplot_width;
var canvas2dplot_height;
function create2DPlotContext(canvas,length){
  canvas2dplot_width = canvas.offsetWidth;
  canvas2dplot_height = canvas.offsetHeight;
  canvas2dplot_length = length;
  return canvas.getContext("2d");
}

function plot2DClear(){
  //Now clear everything out
  canvas2dplot_context.fillStyle = "black";
  canvas2dplot_context.fillRect(0,0,canvas2dplot_width,canvas2dplot_height);
}

function plot2DUpdate(data){
  //First have to start from scratch
  plot2DClear();

  //Now go through and draw a line
  canvas2dplot_context.save();
  canvas2dplot_context.setTransform(1,0,0,1,0,0);
//  canvas2dplot_context.scale(canvas2dplot_width/canvas2dplot_length,canvas2dplot_height); //Why 2?
  canvas2dplot_context.translate(0,canvas2dplot_height);
  canvas2dplot_context.scale(1,-1);
  canvas2dplot_context.beginPath();
  var mult_factor = canvas2dplot_width/canvas2dplot_length;
  canvas2dplot_context.moveTo(0,data[0]*canvas2dplot_height);
  for(var ii=1; ii < canvas2dplot_length; ii++)
    canvas2dplot_context.lineTo(ii*mult_factor,data[ii]*canvas2dplot_height);
  canvas2dplot_context.lineWidth = 2;
  canvas2dplot_context.strokeStyle = "#FFFFFF";
  canvas2dplot_context.stroke();
  canvas2dplot_context.restore();
}

function webgl_plot3d_startup() {
  //Start by setting up the webgl canvas
  canvas = document.getElementById("myGLCanvas");
  //gl = WebGLDebugUtils.makeDebugContext(createGLContext(canvas));
  gl = createGLContext(canvas);
  setupShaders(); 
  setupBuffers(200, 200);
  initPalette();
  gl.clearColor(0.0, 0.0, 0.0, 1.0);
  gl.enable(gl.DEPTH_TEST);
  fpsCounter = document.getElementById("fps");
  
//  gl.frontFace(gl.CCW);
//  gl.enable(gl.CULL_FACE);
//  gl.cullFace(gl.BACK);

  //Next set up the 2D canvas
  canvas2d_context = create2DContext(document.getElementById("my2DWaterfallCanvas"),200,200);
  fountain2DClear();

  //Next set up the 2D plot
  canvas2dplot_context = create2DPlotContext(document.getElementById("my2DPlotCanvas"),200);
  plot2DClear();

}

