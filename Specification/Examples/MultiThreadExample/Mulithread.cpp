class Object(){
	mat4 model;
	texture tex;
	vec3 positions[];
	vec3 color[];
	vec3 uv[];
	vec3 indices[];
}

ThreadPool thread_pool(5);

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
	
	for(auto i = 0; i < thread_pool.thread_count(); ++i) {
		 sBufs[i] = SecondaryCommandBuffer(mainPass);
	}
	
	thread_pool.for_each(objects, [&sBufs](Object& object, size_t thread_index){
	auto& buf = sBufs[thread_index];
	buf.setUniform("model", obj.model);
		buf.setUniform("texture", obj.tex, sampler);
		buf.setVertexBuffer(obj);
		buf.setIndexBuffer(obj.indices);
		buf.drawInstanced(obj.indices.size(), 1, 0, 0);
	});	
	thread_pool.wait();
	
	cbuf1.attatchBuffers({sBuf1});
	mainPass.setCommands({cbuf1});
	rend.renderToScreen(mainPass.output["outColor"]); 
}

