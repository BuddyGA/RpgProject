#include "RpgRenderComponent.h"



RPG_COMPONENT_DEFINITION_STATIC_StreamWrite(RpgRenderComponent_Mesh)
{
	writer.Write(data.bIsVisible);
	writer.Write(data.Bound);
}


RPG_COMPONENT_DEFINITION_STATIC_StreamRead(RpgRenderComponent_Mesh)
{
	reader.Read(data.bIsVisible);
	reader.Read(data.Bound);
}



RPG_COMPONENT_DEFINITION_STATIC_StreamWrite(RpgRenderComponent_Light)
{

}


RPG_COMPONENT_DEFINITION_STATIC_StreamRead(RpgRenderComponent_Light)
{

}



RPG_COMPONENT_DEFINITION_STATIC_StreamWrite(RpgRenderComponent_Camera)
{

}


RPG_COMPONENT_DEFINITION_STATIC_StreamRead(RpgRenderComponent_Camera)
{

}
