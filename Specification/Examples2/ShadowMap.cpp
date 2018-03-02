class object(){
	mat4 depthMVP;
	mat4 MVP;
	mat4 depthBiasMVP;
	vec3 positions[];
	vec3 color[];
	vec3 indices[];
}

std::vector<Object> objects = // Initializer list of objects to draw
auto surface = // Get surface reference

VertexShaderByteCode shadowVertexShader = VertexShader::CompileFromFile("shadow.vert"); 
FragmentShaderByteCode shadowFragmentShader = FragmentShader::CompileFromFile("shadow.frag");
auto shadowPass = RenderPass(shadowVertexShader, shadowFragmentShader);

VertexShaderByteCode vertexShader = VertexShader::CompileFromFile("main.vert"); 
FragmentShaderByteCode fragmentShader = FragmentShader::CompileFromFile("main.frag"); 
auto mainPass = RenderPass(vertexShader, fragmentShader); 

// Creates 3 colorbuffers in a tripple buffer swapchain
Swapchain swapChain = SwapChain(1024, 1024, RGBA, 3, FIFO_BUFFERING, surface);
Image depthBuffer = Image(1024, 1024, RGBA ,IMAGE_DEPTH_BUFFER);
Image shadowDepthBuffer = Image(1024, 1024, RGBA ,IMAGE_DEPTH_BUFFER);

Sampler sampler2d();
sampler.SetMagFilter(NEAREST);
sampler.SetMinFilter(NEAREST);
sampler.SetTextureWrapS(CLAMP_TO_EDGE);
sampler.SetTextureWrapT(CLAMP_TO_EDGE);

auto framebuffers = new std::Vector<Framebuffer>();
for(auto i =0; i < 3 ; ++i){
	framebuffers.push_back({swapChain[i], depthBuffer});
}	

while(true){
	auto index = swapChain.CurrentIndex();
	
	auto cbuf1 = PrimaryCommandBuffer(shadowPass);
	cbuf1.AddPrimitiveTopology(TRIANGLES);
	cBuf1.AddDepthBuffer(shadowDepthBuffer); // How do we set this when we do not have a color buffer?
	cbuf1.AddDepthBufferClear();

	auto sBuf1 = SubCommandBuffer(); 
	for(auto& obj : objects){
		sBuf1.AddUniform("depthMVP", obj.depthMVP);
		sBuf1.AddInput("position", obj.position);
		sBuf1.AddIndexBuffer(obj.indices); 
		sBuf1.AddDrawInstanced(obj.indices.size(), 1, 0, 0);
	}
	cbuf1.attatchBuffers({sBuf1});
	shadowPass.setCommands({cbuf1});

	cbuf2 = PrimaryCommandBuffer(mainPass);
	cbuf2.AddPrimitiveTopology(TRIANGLES);
	cBuf1.AddOutput("outColor", framebuffers[index]);
	cbuf2.AddFrameBufferClear(0.0f, 0.0f, 0.0f, 1.0f);
	cbuf2.AddDepthBufferClear(1.0f);
	cBuf2.AddDepthTest(PPG_LESS);


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
	swapChain.present();
}