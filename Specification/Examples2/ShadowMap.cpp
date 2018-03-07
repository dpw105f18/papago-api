class object(){
	mat4 depthMVP;
	mat4 MVP;
	mat4 depthBiasMVP;
    vector<vec3> positions;
    vector<vec3> colors;
	vector<int> indices;
}

std::vector<Object> objects = {
    // Initializer list of objects to draw
};
Surface surface = Surface( /* Get surface reference */ );
GraphicsQueue graphics_queue = surface.get_graphics_queue();

VertexShader shadowVertexShader = VertexShader::CompileFromFile("shadow.vert"); 
FragmentShader shadowFragmentShader = FragmentShader::CompileFromFile("shadow.frag");
RenderPass shadowPass = RenderPass(shadowVertexShader, shadowFragmentShader);

VertexShader vertexShader = VertexShader::CompileFromFile("main.vert"); 
FragmentShader fragmentShader = FragmentShader::CompileFromFile("main.frag"); 
RenderPass mainPass = RenderPass(vertexShader, fragmentShader); 

// Creates 3 colorbuffers in a tripple buffer swapchain
SwapChain swapChain = SwapChain(RGBA, 3, FIFO_BUFFERING, surface);
Image depthBuffer = Image(surface.width(), surface.height(), S32Float, IMAGE_DEPTH_BUFFER);
Image shadowDepthBuffer = Image(1024, 1024, S32Float, IMAGE_DEPTH_BUFFER);

Sampler2D sampler = Sampler2D();
sampler.SetMagFilter(NEAREST);
sampler.SetMinFilter(NEAREST);
sampler.SetTextureWrapS(CLAMP_TO_EDGE);
sampler.SetTextureWrapT(CLAMP_TO_EDGE);

CommandBuffer cBuf1 = CommandBuffer(shadowPass);
cBuf1.setPrimitiveTopology(TRIANGLES);
cBuf1.setDepthBuffer(shadowDepthBuffer);
cBuf1.clearDepthBuffer();
cBuf1.setDepthTest(PPG_LESS);

SubCommandBuffer sBuf1 = SubCommandBuffer(); 
for(Object& obj : objects){
    sBuf1.setUniform("depthMVP", obj.depthMVP);
    sBuf1.setInput("position", obj.position);
    sBuf1.setIndexBuffer(obj.indices); 
    sBuf1.drawInstanced(obj.indices.size(), 1, 0, 0);
}
cBuf1.attatchBuffers({sBuf1});
shadowPass.setCommands(cBuf1);

cBuf2 = PrimaryCommandBuffer(mainPass);
cBuf2.setPrimitiveTopology(TRIANGLES);
cBuf2.setOutput("outColor", swapchain);
cBuf2.setDepthBuffer(depthBuffer);
cBuf2.clearFrameBuffer(0.0f, 0.0f, 0.0f, 1.0f);
cBuf2.clearDepthBuffer(1.0f);
cBuf2.setDepthTest(PPG_LESS);

cBuf2.setUniform("shadowMap", shadowDepthBuffer, sampler);
auto sBuf2 = SubCommandBuffer(); 
for(auto& obj : objects){	
    sBuf2.setUniform("MVP", obj.MVP);
    sBuf2.setUniform("depthBiasMVP", obj.depthBiasMVP);
    sBuf2.setInput("position", obj.positions);
    sBuf2.setInput("color", obj.colors);
    sBuf2.setIndexBuffer(obj.indices);
    sBuf2.drawInstanced(obj.indices.size(), 1, 0, 0);
}
cbuf2.attatchBuffers({sBuf2});
mainPass.setCommands(cBuf2);

while(true){	
    graphics_queue.submitPass(mainPass);
	swapChain.present();
}