class Object{
	mat4 model;
	texture tex;
	vec3 positions[];
	vec3 color[];
	vec3 uv[];
	vec3 indices[];
}

std::vector<Object> objects = // Initializer list of objects to draw
auto surface = // Get surface reference
mat4 view = // view matrix
mat4 projection = // projection matrix
 
auto mainPass = RenderPass("main.vert", "main.frag");
Renderer rend = Renderer(surface, 3, FIFO_BUFFERING);

Sampler sampler();
sampler.SetMagFilter(NEAREST);
sampler.SetMinFilter(NEAREST);
sampler.SetTextureWrapS(CLAMP_TO_EDGE);
sampler.SetTextureWrapT(CLAMP_TO_EDGE);


while(true)
{
	rend.SetRenderpasses({mainPass}); 
	
	auto cbuf1 = PrimaryCommandBuffer(mainPass);
	cbuf1.setPrimitiveTopology(TRIANGLES);
	cbuf1.clearFrameBuffer(0.0f, 0.0f, 0.0f, 1.0f);
	cbuf1.clearDepthBuffer();
	
	auto sBuf1 = SecondaryCommandBuffer(); 
	sBuf1.setUniform("projection", projection);
	sBuf1.setUniform("view", view);
	
	for(auto& obj : objects){
		sBuf1.setUniform("model", obj.model);
		sBuf1.setUniform("texture", obj.tex, sampler);
		sBuf1.setInput("inPosition", obj.position);
		sBuf1.setInput("inColor", obj.color);
		sBuf1.setInput("inTexCoord", obj.uv);
		sBuf1.setIndexBuffer(obj.indices);
		sBuf1.drawInstanced(obj.indices.size(), 1, 0, 0);
	}
	cbuf1.attatchBuffers({sBuf1});
	mainPass.setCommands({cbuf1});
	rend.renderToScreen(mainPass.output["outColor"]); 
}