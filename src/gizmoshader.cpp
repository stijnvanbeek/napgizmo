/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include "gizmoshader.h"

// Local includes
#include <nap/core.h>

// nap::GizmoShader run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::GizmoShader, "Shader program for rendering gizmos")
	RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	namespace shader
	{
		inline constexpr const char* gizmo = "gizmo";
	}

	GizmoShader::GizmoShader(Core& core) :
		Shader(core)
	{ }


	bool GizmoShader::init(utility::ErrorState& errorState)
	{
		static const char vert_source[] = R"glslang(
			#version 450 core

			uniform nap
			{
				mat4 projectionMatrix;
				mat4 viewMatrix;
				mat4 modelMatrix;
				mat4 normalMatrix;
				vec3 cameraPosition;
			} mvp;

			// Input Vertex Attributes
			in vec3	in_Position;
			in vec3 in_UV0;
			in vec3 in_Normals;
			in vec4 in_Color0;

			// Output to fragment shader
			out vec4 passColor0;
			out float passFresnel;

			void main(void)
			{
				// Calculate frag position
				vec4 world_position = mvp.modelMatrix * vec4(in_Position, 1.0);
			    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

				// Rotate normal based on normal matrix and set
				vec3 normal = normalize(mvp.normalMatrix * vec4(in_Normals, 0.0)).xyz;

				// Calculate mesh to camera angle for halo effect
				vec3 view_dir = normalize(mvp.cameraPosition - world_position.xyz);

				// Dot product gives us the 'angle' between the surface and cam vector
				// The result is that normals pointing away from the camera at an angle of 90* are getting a higer value
				// Normals pointing towards the camera (directly) get a value of 0
				float cam_surface_dot = max(dot(normalize(normal), view_dir), 0.0);
				cam_surface_dot = max((1.0 - cam_surface_dot) + 0.1, 0.0);
				cam_surface_dot = pow(cam_surface_dot, 5.0);

				// Forward to fragment shader
				passFresnel = cam_surface_dot;
				passColor0 = in_Color0;
			}
		)glslang";

		static const char frag_source[] = R"glslang(
			#version 450 core

			// vertex shader input
			in float passFresnel;
			in vec4 passColor0;

			uniform nap
			{
				mat4 projectionMatrix;
				mat4 viewMatrix;
				mat4 modelMatrix;
				mat4 normalMatrix;
				vec3 cameraPosition;
			} mvp;

			// output
			out vec4 out_Color;

			void main()
			{
				// Set fragment color output
				out_Color = vec4(mix(passColor0.rgb, vec3(1.0), passFresnel), 1.0);
			}
		)glslang";

		// Compile shader
		return load(shader::gizmo, {},
			vert_source, sizeof(vert_source)-1,
			frag_source, sizeof(frag_source)-1,
			errorState
		);
	}
}
