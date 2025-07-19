#include "RpgRenderTask_CompilePSO.h"
#include "../RpgRenderPipeline.h"
#include "shader/RpgShaderManager.h"



RpgRenderTask_CompilePSO::RpgRenderTask_CompilePSO() noexcept
{
	RootSignature = nullptr;
}


void RpgRenderTask_CompilePSO::Reset() noexcept
{
	RpgThreadTask::Reset();

	Name = "";
	RootSignature = nullptr;
	PSO.Reset();
}


void RpgRenderTask_CompilePSO::Execute() noexcept
{
	RPG_LogDebug(RpgLogD3D12, "[ThreadId-%u] Execute task compile PSO for (%s)", GetCurrentThreadId(), *Name);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.NodeMask = 0;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	psoDesc.pRootSignature = RootSignature;
	psoDesc.PrimitiveTopologyType = (PipelineState.RasterMode == RpgRenderRasterMode::LINE) ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE : D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// Vertex input elements
	RpgName vertexShaderName;
	RpgArrayInline<D3D12_INPUT_ELEMENT_DESC, 10> vertexInputElements;
	{
		switch (PipelineState.VertexMode)
		{
			case RpgRenderVertexMode::PRIMITIVE_2D:
			{
				vertexShaderName = RPG_SHADER_NAME_Primitive2D_VS;

				vertexInputElements.AddValue({ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
				vertexInputElements.AddValue({ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

				break;
			}

			case RpgRenderVertexMode::GUI:
			{
				vertexShaderName = RPG_SHADER_NAME_GUI_VS;

				vertexInputElements.AddValue({ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
				vertexInputElements.AddValue({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
				vertexInputElements.AddValue({ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

				break;
			}

			case RpgRenderVertexMode::PRIMITIVE:
			{
				vertexShaderName = RPG_SHADER_NAME_Primitive_VS;

				vertexInputElements.AddValue({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
				vertexInputElements.AddValue({ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

				break;
			}

			case RpgRenderVertexMode::MESH:
			{
				vertexShaderName = RPG_SHADER_NAME_VertexMesh;

				// Position
				vertexInputElements.AddValue({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

				// Normal, Tangent
				vertexInputElements.AddValue({ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
				vertexInputElements.AddValue({ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

				// TexCoord
				vertexInputElements.AddValue({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

				break;
			}

			default:
				break;
		}

		psoDesc.InputLayout.pInputElementDescs = vertexInputElements.GetData();
		psoDesc.InputLayout.NumElements = static_cast<UINT>(vertexInputElements.GetCount());
	}


	// Shader state
	{
		// Vertex shader
		if (!PipelineState.VertexShaderName.IsEmpty())
		{
			vertexShaderName = PipelineState.VertexShaderName;
		}

		IDxcBlob* vertexShaderCodeBlob = RpgShaderManager::GetShaderCodeBlob(vertexShaderName);
		RPG_Assert(vertexShaderCodeBlob);
		psoDesc.VS.pShaderBytecode = vertexShaderCodeBlob->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderCodeBlob->GetBufferSize();

		// Geometry shader
		if (!PipelineState.GeometryShaderName.IsEmpty())
		{
			IDxcBlob* geometryShaderCodeBlob = RpgShaderManager::GetShaderCodeBlob(PipelineState.GeometryShaderName);
			RPG_Assert(geometryShaderCodeBlob);
			psoDesc.GS.pShaderBytecode = geometryShaderCodeBlob->GetBufferPointer();
			psoDesc.GS.BytecodeLength = geometryShaderCodeBlob->GetBufferSize();
		}

		// Pixel shader
		if (!PipelineState.PixelShaderName.IsEmpty())
		{
			IDxcBlob* pixelShaderCodeBlob = RpgShaderManager::GetShaderCodeBlob(PipelineState.PixelShaderName);
			RPG_Assert(pixelShaderCodeBlob);
			psoDesc.PS.pShaderBytecode = pixelShaderCodeBlob->GetBufferPointer();
			psoDesc.PS.BytecodeLength = pixelShaderCodeBlob->GetBufferSize();
		}
	}


	// Rasterizer state
	{
		psoDesc.RasterizerState.CullMode = PipelineState.bTwoSides ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
		psoDesc.RasterizerState.DepthBias = PipelineState.DepthBias;
		psoDesc.RasterizerState.SlopeScaledDepthBias = PipelineState.DepthBiasSlope;
		psoDesc.RasterizerState.DepthBiasClamp = PipelineState.DepthBiasClamp;
		psoDesc.RasterizerState.DepthClipEnable = PipelineState.bDepthTest;
		psoDesc.RasterizerState.AntialiasedLineEnable = PipelineState.RasterMode == RpgRenderRasterMode::LINE;
		psoDesc.RasterizerState.MultisampleEnable = FALSE;
		psoDesc.RasterizerState.ForcedSampleCount = 0;
		psoDesc.RasterizerState.ConservativeRaster = PipelineState.bConservativeRasterization ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		psoDesc.RasterizerState.FillMode = (PipelineState.RasterMode == RpgRenderRasterMode::SOLID) ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
	}


	// Render target state
	psoDesc.NumRenderTargets = PipelineState.RenderTargetCount;
	psoDesc.BlendState.AlphaToCoverageEnable = FALSE;
	const RpgRenderColorBlendMode blendMode = PipelineState.BlendMode;
	const DXGI_FORMAT renderTargetFormat = PipelineState.RenderTargetFormat != DXGI_FORMAT_UNKNOWN ? PipelineState.RenderTargetFormat : RpgRender::DEFAULT_FORMAT_SCENE_RENDER_TARGET;

	for (UINT r = 0; r < psoDesc.NumRenderTargets; ++r)
	{
		psoDesc.RTVFormats[r] = renderTargetFormat;

		D3D12_RENDER_TARGET_BLEND_DESC& renderTargetBlendDesc = psoDesc.BlendState.RenderTarget[r];
		renderTargetBlendDesc.BlendEnable = FALSE;
		renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		switch (blendMode)
		{
			case RpgRenderColorBlendMode::OPACITY_MASK:
			{
				renderTargetBlendDesc.BlendEnable = TRUE;
				renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
				renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
				renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_MAX;
				renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;

				break;
			}

			case RpgRenderColorBlendMode::FADE:
			{
				renderTargetBlendDesc.BlendEnable = TRUE;
				renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
				renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
				renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

				break;
			}

			case RpgRenderColorBlendMode::TRANSPARENCY:
			{
				renderTargetBlendDesc.BlendEnable = TRUE;
				renderTargetBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
				renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
				renderTargetBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
				renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
				renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

				break;
			}

			default:
				break;
		}
	}


	// Depth stencil state
	{
		DXGI_FORMAT depthStencilFormat = PipelineState.DepthStencilFormat;

		if (depthStencilFormat == DXGI_FORMAT_UNKNOWN)
		{
			depthStencilFormat = PipelineState.bStencilTest ? DXGI_FORMAT_D24_UNORM_S8_UINT : DXGI_FORMAT_D32_FLOAT;
		}

		psoDesc.DSVFormat = depthStencilFormat;
		psoDesc.DepthStencilState.DepthEnable = PipelineState.bDepthTest;
		psoDesc.DepthStencilState.DepthWriteMask = PipelineState.bDepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		psoDesc.DepthStencilState.StencilEnable = PipelineState.bStencilTest;
	}


	// Sample state
	{
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.SampleMask = UINT32_MAX;
	}

	RPG_D3D12_Validate(RpgD3D12::GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&PSO)));
	RPG_D3D12_SetDebugName(PSO, "PSO_%s", *Name);

	RPG_LogDebug(RpgLogD3D12, "[ThreadId-%u] Compiled PSO (PSO_%s)", GetCurrentThreadId(), *Name);
}
