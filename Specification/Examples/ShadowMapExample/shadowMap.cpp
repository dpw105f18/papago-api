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

auto shadowPass = RenderPass("shadow.vert", "shadow.frag");
auto mainPass = RenderPass("main.vert", "main.frag");
Renderer rend =  Renderer(surface, 3, FIFO_BUFFERING);

Sampler sampler();
sampler.SetMagFilter(NEAREST);
sampler.SetMinFilter(NEAREST);
sampler.SetTextureWrapS(CLAMP_TO_EDGE);
sampler.SetTextureWrapT(CLAMP_TO_EDGE);


while(true){
	rend.SetRenderpasses({shadowPass, mainPass}); 
	
	auto cbuf1 = PrimaryCommandBuffer(shadowPass);
	cbuf1.setPrimitiveTopology(TRIANGLES);
	cbuf1.clearFrameBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	cbuf1.clearDepthBuffer();
	
	auto sBuf1 = SecondaryCommandBuffer(); 
	for(auto& obj : objects){
		sBuf1.setUniform("depthMVP", obj.depthMVP);
		sBuf1.setInput("position", obj.position);
		sBuf1.setIndexBuffer(obj.indices); 
		sBuf1.drawInstanced(obj.indices.size(), 1, 0, 0);
	}
	cbuf1.attatchBuffers({sBuf1});
	shadowPass.setCommands({cbuf1});
	
	cbuf2 = PrimaryCommandBuffer(mainPass);
	cbuf2.setPrimitiveTopology(TRIANGLES);
	cbuf2.clearFrameBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	cbuf2.clearDepthBuffer();
	cBuf2.setUniform("shadowMap", shadowPass.output["fragmentdepth"], sampler);
	
	auto sBuf2 = SecondaryCommandBuffer(); 
	for(auto& obj : objects){	
		sBuf2.setUniform("MVP", obj.MVP);
		sBuf2.setUniform("depthBiasMVP", obj.depthBiasMVP);
		sBuf2.setInput("position", obj.position);
		sBuf2.setInput("color", obj.color);
		sBuf2.indexBuffer(obj.indices);
		sBuf2.drawInstanced(obj.indices.size(), 1, 0, 0);
	}
	cbuf2.attatchBuffers({sBuf2});
	mainPass.setCommands({cBuf2});
	
	rend.renderToScreen(mainPass.output["outColor"]);
}