#include <maya/MFnPlugin.h>
#include <maya/MDrawRegistry.h>
#include "AbcShape.h"
#include "AbcShapeImport.h"
#include "VP2.h"

PLUGIN_EXPORT MStatus initializePlugin(MObject obj)
{
   const char * pluginVersion = "1.0";
   
   MString nodeName = PREFIX_NAME("AbcShape");
   MString commandName = PREFIX_NAME("AbcShapeImport");
   
   MFnPlugin plugin(obj, nodeName.asChar(), pluginVersion, "Any");

   MStatus status = plugin.registerShape(nodeName,
                                         AbcShape::ID,
                                         AbcShape::creator,
                                         AbcShape::initialize,
                                         AbcShapeUI::creator,
                                         &AbcShapeOverride::Classification);
   if (status != MS::kSuccess)
   {
      status.perror("Failed to register shape '" + nodeName + "'");
      return status;
   }
   
   status = MHWRender::MDrawRegistry::registerDrawOverrideCreator(AbcShapeOverride::Classification,
                                                                  AbcShapeOverride::Registrant,
                                                                  AbcShapeOverride::create);
   
   if (status != MS::kSuccess)
   {
      status.perror("Failed to register draw override for '" + nodeName + "'");
      return status;
   }
   
   status = plugin.registerCommand(commandName, AbcShapeImport::create, AbcShapeImport::createSyntax);
   
   if (status != MS::kSuccess)
   {
      status.perror("Failed to register command '" + commandName + "'");
      return status;
   }
   
   return status;
}

PLUGIN_EXPORT MStatus uninitializePlugin(MObject obj)
{
   MFnPlugin plugin(obj);

   MString nodeName = PREFIX_NAME("AbcShape");
   MString commandName = PREFIX_NAME("AbcShapeImport");

   if (AbcShape::CallbackID != 0)
   {
      MDGMessage::removeCallback(AbcShape::CallbackID);
      AbcShape::CallbackID = 0;
   }
   
   MStatus status = plugin.deregisterCommand(commandName);
   
   if (status != MS::kSuccess)
   {
      status.perror("Failed to deregister command '" + commandName + "'");
      return status;
   }

   status = MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(AbcShapeOverride::Classification,
                                                                    AbcShapeOverride::Registrant);
   
   if (status != MS::kSuccess)
   {
      status.perror("Failed to deregister draw override for '" + nodeName + "'");
      return status;
   }
   
   status = plugin.deregisterNode(AbcShape::ID);
   
   if (status != MS::kSuccess)
   {
      status.perror("Failed to deregister shape '" + nodeName + "'");
      return status;
   }
   
   return status;
}
