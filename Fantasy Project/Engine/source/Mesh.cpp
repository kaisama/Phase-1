#include <Mesh.h>
#include <stdint.h>
#include <Components.h>

static bool Glew_Initialized = false;

//Create the buffers and store the mesh data in them
Mesh CreateMesh(Vertex *vertices, unsigned int verticesCount, unsigned int *indices, unsigned int indicesCount, bool withNormals, bool batch)
{
	Mesh mesh = {};
	if (!Glew_Initialized){
		bool res = ReloadGlew();
		Glew_Initialized = true;
	}

	mesh.IndicesCount = indicesCount;

	if (withNormals)
	{
		CalculateNormals(vertices, verticesCount, indices, indicesCount);
	}

	//Use only with newer GLSL versions
#if GLSL_VERSION == MODERN_VERSION
	if (!buffers->VAO)
	{
		//Generate a handle to a vertex array object
		glGenVertexArrays(1, &buffers->VAO);
	}
#endif
	/***********************************************************************************************************/
	//Create a Vertex buffer
	//NOTE(kai): We create the vertex buffer by:
	//					1) Create a handle for the buffer
	//					2) Bind the buffer
	//					3) Store the data in the buffer
	//					4) Specify the vertex layout by enabling the attributes in the shader
	//						,set the attribute pointer
	/***********************************************************************************************************/

	/***********************************************************************************************************/
	//Create an Element buffer
	//NOTE(kai): We create the element buffer by:
	//					1) Create a handle for the buffer
	//					2) Bind the buffer
	//					3) Store the data in the buffer
	/***********************************************************************************************************/

	//Generate a handle to a vertex buffer object
	glGenBuffers(1, &mesh.VBO);
	//Generate a handle to a element buffer object
	glGenBuffers(1, &mesh.EBO);

	BindMesh(&mesh);


	if (!batch)
	{
		//Store the data in the buffer
		glBufferData(GL_ARRAY_BUFFER, verticesCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

		//Store the data in the buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	}
	else
	{
		//Store the data in the buffer
		glBufferData(GL_ARRAY_BUFFER, verticesCount * sizeof(Vertex), vertices, GL_DYNAMIC_DRAW);

		//Store the data in the buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesCount * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);
	}

	/***********************************************************************************************************/
	/***********************************************************************************************************/

	//Unbind everything
	UnbindMesh();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return mesh;
}

//Bind the buffers
void BindMesh(Mesh *mesh)
{
	//Use only with newer GLSL versions
#if GLSL_VERSION == MODERN_VERSION

	//Bind the element buffer
	glBindVertexArray(buffers->VAO);

#elif GLSL_VERSION == ANCIENT_VERSION	//Use with older GLSL versions
	//Bind the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

	//Enable the attribute with the given index in the shader to be used in rendering
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	//Defines where the attribute with the given index should look in buffer
	glVertexAttribPointer(0				//The attribute index
		, 3				//Number of elements
		, GL_FLOAT		//Type of elements
		, GL_FALSE		//Normalized or not
		, sizeof(Vertex)//Size of the stride (the next location that the pointer will go to)
		, (void *)offsetof(Vertex, Vertex::Pos));			//The pointer offset

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Vertex::TexCoords));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Vertex::Color));
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, Vertex::TextureSlot)));

	//Bind the vertex buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->Debug_EBO);
#endif
}

//Unbind the buffers
void UnbindMesh()
{
	//Use only with newer GLSL versions
#if GLSL_VERSION == MODERN_VERSION

	//Unbind the vertex array object
	glBindVertexArray(0);

#endif
	//Unbind the vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Unbind the element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//Draw the mesh
void DrawMesh(Mesh *mesh, bool debug)
{
	if (debug)
	{
		glLineWidth(1.0f);
		BindMesh(mesh);
		glDrawElements(GL_LINES, mesh->IndicesCount, GL_UNSIGNED_INT, NULL);
		UnbindMesh();
	}
	else
	{
		//Bind the buffers
		BindMesh(mesh);

		//Draw the mesh
		glDrawElements(GL_TRIANGLES		//Drawing type
			, mesh->IndicesCount	//Number of indices
			, GL_UNSIGNED_INT	//Indices type
			, 0);				//The location of the indices (NULL means to look for them in the buffer) 

		UnbindMesh();
	}
}

////
Mesh CreateSprite(vec3f pos, vec2f size, vec4f color, AnimationClip *clip, bool debug)
{
	Mesh sprite = {};

	if (clip)
	{
		uint32 col = clip->Indices[clip->Counter] % clip->MaxCountHorizontal;
		uint32 row = (clip->MaxCountVertical - 1) - (clip->Indices[clip->Counter] / clip->MaxCountHorizontal);

		vec2f bottomLeft = vec2f(float(col) * clip->FrameWidth, float(row) * clip->FrameHeight);
		vec2f bottomRight = vec2f((float(col) * clip->FrameWidth) + clip->FrameWidth, float(row) * clip->FrameHeight);
		vec2f topLeft = vec2f(float(col) * clip->FrameWidth, (float(row) * clip->FrameHeight) + clip->FrameHeight);
		vec2f topRight = vec2f((float(col) * clip->FrameWidth) + clip->FrameWidth, (float(row) * clip->FrameHeight) + clip->FrameHeight);

		vec3f lowerLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
		vec3f upperLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
		vec3f lowerRight = vec3f(pos.X + (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
		vec3f upperRight = vec3f(pos.X + (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);

		Vertex vertices[] =
		{
			Vertex{ lowerLeft, bottomLeft, color, 0 },
			Vertex{ upperLeft, topLeft, color, 0 },
			Vertex{ upperRight, topRight, color, 0 },
			Vertex{ lowerRight, bottomRight, color, 0 }
		};

		if (debug)
		{
			uint32 indices[] =
			{
				0,
				1,

				1,
				2,

				2,
				3,

				3,
				0,
			};

			sprite = CreateMesh(vertices, 4, indices, 8, 0);
		}
		else
		{
			unsigned int indices[] =
			{
				0, 1, 3,
				1, 2, 3,
			};

			sprite = CreateMesh(vertices, 4, indices, 6, 0);
		}
	}
	else
	{
		vec3f lowerLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
		vec3f upperLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
		vec3f lowerRight = vec3f(pos.X + (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
		vec3f upperRight = vec3f(pos.X + (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);

		Vertex vertices[] =
		{
			Vertex{ lowerLeft, vec2f(0, 0), color, 0 },
			Vertex{ upperLeft, vec2f(0, 1), color, 0 },
			Vertex{ upperRight, vec2f(1, 1), color, 0 },
			Vertex{ lowerRight, vec2f(1, 0), color, 0 }
		};

		if (debug)
		{
			uint32 indices[] =
			{
				0,
				1,

				1,
				2,

				2,
				3,

				3,
				0,
			};

			sprite = CreateMesh(vertices, 4, indices, 8, 0);
		}
		else
		{
			unsigned int indices[] =
			{
				0, 1, 3,
				1, 2, 3,
			};

			sprite = CreateMesh(vertices, 4, indices, 6, 0);
		}
	}	

	return sprite;
}

void AnimateSprite(Mesh *sprite, AnimationClip *clip)
{
	BindMesh(sprite);

	Vertex *vertices = (Vertex *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	if (clip->Loop)
	{
		if (clip->Counter >= clip->FrameCount)
		{
			clip->Counter = 0;
		}
	}

	if (clip->Counter < clip->FrameCount)
	{
		if (clip->TimeElapsed > clip->RunSpeed_FPS)
		{
			clip->TimeElapsed = 0;

			uint32 col = clip->Indices[clip->Counter] % clip->MaxCountHorizontal;
			uint32 row = (clip->MaxCountVertical - 1) - (clip->Indices[clip->Counter] / clip->MaxCountHorizontal);

			vec2f bottomLeft = vec2f(float(col) * clip->FrameWidth, float(row) * clip->FrameHeight);
			vec2f bottomRight = vec2f((float(col) * clip->FrameWidth) + clip->FrameWidth, float(row) * clip->FrameHeight);
			vec2f topLeft = vec2f(float(col) * clip->FrameWidth, (float(row) * clip->FrameHeight) + clip->FrameHeight);
			vec2f topRight = vec2f((float(col) * clip->FrameWidth) + clip->FrameWidth, (float(row) * clip->FrameHeight) + clip->FrameHeight);

			vertices[0].TexCoords = bottomLeft;
			vertices[1].TexCoords = topLeft;
			vertices[2].TexCoords = topRight;
			vertices[3].TexCoords = bottomRight;

			clip->Counter++;
		}
		clip->TimeElapsed += (1.0f / 60.0f);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	UnbindMesh();
}

void EditSprite(Mesh *sprite, vec3f pos, vec2f size, vec4f color)
{
	BindMesh(sprite);

	Vertex *vertices = (Vertex *)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);

	vec3f lowerLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
	vec3f upperLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
	vec3f lowerRight = vec3f(pos.X + (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
	vec3f upperRight = vec3f(pos.X + (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);

	vertices[0].Pos = lowerLeft; 
	vertices[0].Color = color;

	vertices[1].Pos = upperLeft;
	vertices[1].Color = color;

	vertices[2].Pos = upperRight;
	vertices[2].Color = color;

	vertices[3].Pos = lowerRight;
	vertices[3].Color = color;

	glUnmapBuffer(GL_ARRAY_BUFFER);
	UnbindMesh();
}

Mesh CreateCube(vec3f pos, vec3f size, vec4f color, bool withNormals)
{
	Vertex vertices[24] =
	{
		Vertex{ vec3f(pos.X, pos.Y, pos.Z), vec2f(0, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y, pos.Z), vec2f(1, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y + size.Y, pos.Z), vec2f(1, 1), color },
		Vertex{ vec3f(pos.X, pos.Y + size.Y, pos.Z), vec2f(0, 1), color },

		Vertex{ vec3f(pos.X, pos.Y + size.Y, pos.Z), vec2f(0, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y + size.Y, pos.Z), vec2f(1, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y + size.Y, pos.Z - size.Z), vec2f(1, 1), color },
		Vertex{ vec3f(pos.X, pos.Y + size.Y, pos.Z - size.Z), vec2f(0, 1), color },

		Vertex{ vec3f(pos.X, pos.Y, pos.Z - size.Z), vec2f(0, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y, pos.Z - size.Z), vec2f(1, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y + size.Y, pos.Z - size.Z), vec2f(1, 1), color },
		Vertex{ vec3f(pos.X, pos.Y + size.Y, pos.Z - size.Z), vec2f(0, 1), color },

		Vertex{ vec3f(pos.X, pos.Y, pos.Z), vec2f(0, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y, pos.Z), vec2f(1, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y, pos.Z - size.Z), vec2f(1, 1), color },
		Vertex{ vec3f(pos.X, pos.Y, pos.Z - size.Z), vec2f(0, 1), color },

		Vertex{ vec3f(pos.X, pos.Y, pos.Z - size.Z), vec2f(0, 0), color },
		Vertex{ vec3f(pos.X, pos.Y, pos.Z), vec2f(1, 0), color },
		Vertex{ vec3f(pos.X, pos.Y + size.Y, pos.Z), vec2f(1, 1), color },
		Vertex{ vec3f(pos.X, pos.Y + size.Y, pos.Z - size.Z), vec2f(0, 1), color },

		Vertex{ vec3f(pos.X + size.X, pos.Y, pos.Z - size.Z), vec2f(0, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y, pos.Z), vec2f(1, 0), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y + size.Y, pos.Z), vec2f(1, 1), color },
		Vertex{ vec3f(pos.X + size.X, pos.Y + size.Y, pos.Z - size.Z), vec2f(0, 1), color },
	};

	uint32 indices[36] =
	{
		0, 1, 2,
		2, 0, 3,

		0 + 4, 1 + 4, 2 + 4,
		2 + 4, 0 + 4, 3 + 4,

		0 + 8, 1 + 8, 2 + 8,
		2 + 8, 0 + 8, 3 + 8,

		0 + 12, 1 + 12, 2 + 12,
		2 + 12, 0 + 12, 3 + 12,

		0 + 16, 1 + 16, 2 + 16,
		2 + 16, 0 + 16, 3 + 16,

		0 + 20, 1 + 20, 2 + 20,
		2 + 20, 0 + 20, 3 + 20,
	};

	Mesh result = CreateMesh(vertices, 24, indices, 36, withNormals);

	return result;
}

void CalculateNormals(Vertex *vertices, unsigned int verticesCount, unsigned int *indices, unsigned int indicesCount)
{
	/*for (uint32 i = 0; i < indicesCount; i += 3)
	{
	uint32 i0 = indices[i];
	uint32 i1 = indices[i + 1];
	uint32 i2 = indices[i + 2];

	vec3f v1 = vertices[i1].Pos - vertices[i0].Pos;
	vec3f v2 = vertices[i2].Pos - vertices[i0].Pos;

	vec3f normal = v1.Cross(v2);
	normal.Normalize();

	vertices[i0].Normal += normal;
	vertices[i1].Normal += normal;
	vertices[i2].Normal += normal;

	vertices[i0].Normal.Normalize();
	vertices[i1].Normal.Normalize();
	vertices[i2].Normal.Normalize();
	}*/
}

void BeginBatch(MeshBatch *batch, BATCH_TYPE type, uint32 maxCount, bool debug)
{
	batch->MaxCount = maxCount;

	if (batch->Type == BATCH_TYPE::UNKNOWN && type != BATCH_TYPE::UNKNOWN)
	{
		batch->Type = type;
	}

	if (type = BATCH_TYPE::SPRITE_BATCH)
	{
		if (debug)
		{
			if (!batch->Batch.VBO)
			{
				batch->Batch = CreateMesh(NULL, 4 * batch->MaxCount, NULL, 8 * batch->MaxCount, false, true);
			}

			BindMesh(&batch->Batch);

			//Store the data in the buffer
			glBufferData(GL_ARRAY_BUFFER, 4 * batch->MaxCount * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

			//Store the data in the buffer
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 8 * batch->MaxCount * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
		}
		else
		{
			if (!batch->Batch.VBO)
			{
				batch->Batch = CreateMesh(NULL, 4 * batch->MaxCount, NULL, 6 * batch->MaxCount, false, true);
			}

			BindMesh(&batch->Batch);

			//Store the data in the buffer
			glBufferData(GL_ARRAY_BUFFER, 4 * batch->MaxCount * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

			//Store the data in the buffer
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * batch->MaxCount * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
		}
	}
	else
	{
		if (!batch->Batch.VBO)
		{
			batch->Batch = CreateMesh(NULL, 24 * batch->MaxCount, NULL, 36 * batch->MaxCount, false, true);
		}

		BindMesh(&batch->Batch);

		//Store the data in the buffer
		glBufferData(GL_ARRAY_BUFFER, 24 * batch->MaxCount * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

		//Store the data in the buffer
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * batch->MaxCount * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
	}
}

void PauseBatch(MeshBatch *batch)
{
	UnbindMesh();
}

void ResumeBatch(MeshBatch *batch)
{
	BindMesh(&batch->Batch);

	for (uint32 i = 0; i < batch->UsedSlotsCount; i++)
	{
		batch->TextureSlots[i] = 0;
	}

	batch->UsedSlotsCount = 0;
}

void AddSprite(MeshBatch *batch, vec3f pos, vec2f size, vec4f color, uint32 textureID, AnimationClip *clip, bool debug, mat4f model)
{
	uint32 vertexOffset = batch->CurrentSize * 4;
	uint32 indexOffset = batch->CurrentSize * 6;
	if (debug)
	{
		indexOffset = batch->CurrentSize * 8;
	}

	batch->CurrentSize++;

	bool found = false;
	float slotIndex = 0;

	for (uint32 i = 0; i < batch->UsedSlotsCount; i++)
	{
		if (batch->TextureSlots[i] == textureID)
		{
			found = true;
			slotIndex = i;
			break;
		}
	}

	if (!found)
	{
		if (batch->UsedSlotsCount >= 8)
		{
			PauseBatch(batch);
			RenderBatch(batch, debug);
			ResumeBatch(batch);
		}

		batch->TextureSlots[batch->UsedSlotsCount] = textureID;
		slotIndex = batch->UsedSlotsCount;
		batch->UsedSlotsCount++;
	}
	if (!debug)
	{
		if (clip)
		{
			if (clip->Loop)
			{
				if (clip->Counter >= clip->FrameCount)
				{
					clip->Counter = 0;
				}
			}

			if (clip->Counter < clip->FrameCount)
			{
				if (clip->TimeElapsed > clip->RunSpeed_FPS)
				{
					clip->TimeElapsed = 0;

					uint32 col = clip->Indices[clip->Counter] % clip->MaxCountHorizontal;
					uint32 row = (clip->MaxCountVertical - 1) - (clip->Indices[clip->Counter] / clip->MaxCountHorizontal);

					vec2f bottomLeft = vec2f(float(col) * clip->FrameWidth, float(row) * clip->FrameHeight);
					vec2f bottomRight = vec2f((float(col) * clip->FrameWidth) + clip->FrameWidth, float(row) * clip->FrameHeight);
					vec2f topLeft = vec2f(float(col) * clip->FrameWidth, (float(row) * clip->FrameHeight) + clip->FrameHeight);
					vec2f topRight = vec2f((float(col) * clip->FrameWidth) + clip->FrameWidth, (float(row) * clip->FrameHeight) + clip->FrameHeight);

					vec3f lowerLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
					vec3f upperLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
					vec3f lowerRight = vec3f(pos.X + (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
					vec3f upperRight = vec3f(pos.X + (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);

					Vertex vertices[] =
					{
						Vertex{ lowerLeft, bottomLeft, color, slotIndex },
						Vertex{ upperLeft, topLeft, color, slotIndex },
						Vertex{ upperRight, topRight, color, slotIndex },
						Vertex{ lowerRight, bottomRight, color, slotIndex }
					};
					glBufferSubData(GL_ARRAY_BUFFER, vertexOffset * sizeof(Vertex), 4 * sizeof(Vertex), vertices);
					clip->Counter++;
				}
				clip->TimeElapsed += (1.0f / 60.0f);
			}
		}
		else
		{
			vec3f lowerLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
			vec3f upperLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
			vec3f lowerRight = vec3f(pos.X + (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
			vec3f upperRight = vec3f(pos.X + (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
			
			Vertex vertices[] =
			{
				Vertex{ lowerLeft, vec2f(0, 0), color, slotIndex },
				Vertex{ upperLeft, vec2f(0, 1), color, slotIndex },
				Vertex{ upperRight, vec2f(1, 1), color, slotIndex },
				Vertex{ lowerRight, vec2f(1, 0), color, slotIndex }
			};
			glBufferSubData(GL_ARRAY_BUFFER, vertexOffset * sizeof(Vertex), 4 * sizeof(Vertex), vertices);
		}
	}
	else
	{
		vec3f lowerLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
		lowerLeft = model * lowerLeft;
		vec3f upperLeft = vec3f(pos.X - (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
		upperLeft = model * upperLeft;
		vec3f lowerRight = vec3f(pos.X + (size.X / 2.0f), pos.Y - (size.Y / 2.0f), pos.Z);
		lowerRight = model * lowerRight;
		vec3f upperRight = vec3f(pos.X + (size.X / 2.0f), pos.Y + (size.Y / 2.0f), pos.Z);
		upperRight = model * upperRight;

		Vertex vertices[] =
		{
			Vertex{ lowerLeft, vec2f(0, 0), color },
			Vertex{ upperLeft, vec2f(0, 1), color },
			Vertex{ upperRight, vec2f(1, 1), color },
			Vertex{ lowerRight, vec2f(1, 0), color }
		};
		glBufferSubData(GL_ARRAY_BUFFER, vertexOffset * sizeof(Vertex), 4 * sizeof(Vertex), vertices);
	}

	if (debug)
	{
		uint32 indices[] =
		{
			0 + vertexOffset,
			1 + vertexOffset,

			1 + vertexOffset,
			2 + vertexOffset,

			2 + vertexOffset,
			3 + vertexOffset,

			3 + vertexOffset,
			0 + vertexOffset,
		};

		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffset * sizeof(uint32), 8 * sizeof(uint32), indices);
	}
	else
	{
		uint32 indices[] =
		{
			0 + vertexOffset,
			1 + vertexOffset,
			3 + vertexOffset,

			1 + vertexOffset,
			2 + vertexOffset,
			3 + vertexOffset
		};

		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffset * sizeof(uint32), 6 * sizeof(uint32), indices);
	}
}

void EndBatch(MeshBatch *batch, bool debug)
{
	UnbindMesh();
	RenderBatch(batch, debug);
	batch->CurrentSize = 0;
}

void RenderBatch(MeshBatch *batch, bool debug)
{
	for (uint32 i = 0; i < batch->UsedSlotsCount; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, batch->TextureSlots[i]);
	}

	if (debug)
	{
		glLineWidth(1.0f);
		BindMesh(&batch->Batch);
		glDrawElements(GL_LINES, batch->Batch.IndicesCount, GL_UNSIGNED_INT, NULL);
		UnbindMesh();
	}
	else
	{
		DrawMesh(&batch->Batch, false);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}
