class object(){
	mat4 depthMVP;
	mat4 MVP;
	mat4 depthBiasMVP;
	vec3 positions[];
	vec3 color[];
	vec3 indices[];
}

std::vector<Object> objects = // Initializer list of objects to draw
Surface surface = // Get surface reference

VertexShader shadowVertexShader = VertexShader::CompileFromFile("shadow.vert"); 
FragmentShader shadowFragmentShader = FragmentShader::CompileFromFile("shadow.frag");
RenderPass shadowPass = RenderPass(shadowVertexShader, shadowFragmentShader);

VertexShader vertexShader = VertexShader::CompileFromFile("main.vert"); 
FragmentShader fragmentShader = FragmentShader::CompileFromFile("main.frag"); 
RenderPass mainPass = RenderPass(vertexShader, fragmentShader); 

// Creates 3 colorbuffers in a tripple buffer swapchain
SwapChain swapChain = SwapChain(1024, 1024, RGBA, 3, FIFO_BUFFERING, surface);
Image depthBuffer = Image(1024, 1024, S32Float, IMAGE_DEPTH_BUFFER);
Image shadowDepthBuffer = Image(1024, 1024, S32Float, IMAGE_DEPTH_BUFFER);

Sampler2D sampler = Sampler2D();
sampler.SetMagFilter(NEAREST);
sampler.SetMinFilter(NEAREST);
sampler.SetTextureWrapS(CLAMP_TO_EDGE);
sampler.SetTextureWrapT(CLAMP_TO_EDGE);

std::vector<Framebuffer> framebuffers = std::vector<Framebuffer>();
for(int i = 0; i < 3 ; ++i){
	framebuffers.push_back({swapChain[i], depthBuffer});
}

while(true){
	int index = swapChain.CurrentIndex();
	
	CommandBuffer cBuf1 = CommandBuffer(shadowPass);
	cBuf1.setPrimitiveTopology(TRIANGLES);
	cBuf1.setDepthBuffer(shadowDepthBuffer); // How do we set this when we do not have a color buffer?
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
	shadowPass.setCommands({cBuf1});

	cBuf2 = PrimaryCommandBuffer(mainPass);
	cBuf2.setPrimitiveTopology(TRIANGLES);
	cBuf2.setOutput("outColor", framebuffers[index]);
	cBuf2.clearFrameBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	cBuf2.clearDepthBuffer(1.0f);
	cBuf2.setDepthTest(PPG_LESS);

	cBuf2.setUniform("shadowMap", shadowDepthBuffer, sampler);
	auto sBuf2 = SubCommandBuffer(); 
	for(auto& obj : objects){	
		sBuf2.AddUniform("MVP", obj.MVP);
		sBuf2.AddUniform("depthBiasMVP", obj.depthBiasMVP);
		sBuf2.AddInput("position", obj.position);
		sBuf2.AddInput("color", obj.color);
		sBuf2.AddIndexBuffer(obj.indices);
		sBuf2.AddDrawInstanced(obj.indices.size(), 1, 0, 0);
	}
	cbuf2.attatchBuffers({sBuf2});
	mainPass.setCommands({cBuf2});
    swapChain.submitPass(mainPass);
	swapChain.present();
}