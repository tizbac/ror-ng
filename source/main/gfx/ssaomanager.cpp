/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ssaomanager.h"


#include "SkyManager.h"

#include "Ogre.h"
#include "OgreShadowCameraSetup.h"
#include "OgreTerrain.h"
#include "OgreTerrainMaterialGeneratorA.h"

#include "Settings.h"
#include <TerrainManager.h>
#include <TerrainGeometryManager.h>

using namespace Ogre;


SSAOManager::SSAOManager()
{
    mWidth = gEnv->viewPort->getActualWidth();
    mHeight = gEnv->viewPort->getActualHeight();
    
    mDepthTexture.setNull();
    mDepthMaterial.setNull();

    createDepthRenderTexture();
}

SSAOManager::~SSAOManager()
{

}

void SSAOManager::loadConfiguration()
{
   
    if ( SSETTING("SSAO","enabled") != "enabled" )
        return;
  
    CompositorInstance * inst = CompositorManager::getSingleton().addCompositor(gEnv->viewPort,"SSAO");
    if ( inst )
        inst->setEnabled(true);
    else
        LogManager::getSingleton().logMessage("Failed to create ssao compositor");
        
    LogManager::getSingleton().logMessage("SSAO Initialized\n");
}

void SSAOManager::addCompositor()
{

}

void SSAOManager::createDepthRenderTexture()
{
    // Create the depth render texture
    mDepthTexture = TextureManager::getSingleton().createManual(
        "DoF_Depth",ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        TEX_TYPE_2D, mWidth, mHeight,
        0, Ogre::PF_FLOAT32_RGBA, TU_RENDERTARGET);
    mPositionTexture = TextureManager::getSingleton().createManual(
        "SSAO_Pos",ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        TEX_TYPE_2D, mWidth, mHeight,
        0, Ogre::PF_FLOAT32_RGBA, TU_RENDERTARGET);
    // Get its render target and add a viewport to it
    
    mrt = Ogre::Root::getSingleton().getRenderSystem()->createMultiRenderTarget("SSAOMRT");
    mrt->bindSurface(0,mDepthTexture->getBuffer()->getRenderTarget());
    mrt->bindSurface(1,mPositionTexture->getBuffer()->getRenderTarget());
    
   // mDepthTarget = mDepthTexture->getBuffer()->getRenderTarget();
    mDepthViewport = mrt->addViewport(gEnv->mainCamera);

    // Register 'this' as a render target listener
    mrt->addListener(this);

    // Get the technique to use when rendering the depth render texture
    mTerrainDepthMaterial = MaterialManager::getSingleton().getByName("terraindepth");
    mTerrainDepthMaterial->load();
    mTerrainTech = mTerrainDepthMaterial->getBestTechnique();
    if ( mTerrainDepthMaterial.isNull() )
        abort();
    if ( ! mTerrainTech )
        abort();
    mDepthMaterial = MaterialManager::getSingleton().getByName("DoF_Depth");
    mDepthMaterial->load(); // needs to be loaded manually
    mDepthTechnique = mDepthMaterial->getBestTechnique();

    // Create a custom render queue invocation sequence for the depth render texture
    RenderQueueInvocationSequence* invocationSequence = Root::getSingleton().createRenderQueueInvocationSequence("DoF_Depth");

    // Add a render queue invocation to the sequence, and disable shadows for it
    /*
    RenderQueueInvocation* invocation = invocationSequence->add(RENDER_QUEUE_MAIN, "main");
    invocation->setSuppressShadows(true);
    //invocation->setSuppressRenderStateChanges(true);
    invocation->setSolidsOrganisation(QueuedRenderableCollection::OM_SORT_ASCENDING);
    // Set the render queue invocation sequence for the depth render texture viewport
    mDepthViewport->setRenderQueueInvocationSequenceName("DoF_Depth");
    */

    mDepthViewport->setShadowsEnabled(false);
    mDepthViewport->setOverlaysEnabled(false);
    mDepthViewport->setSkiesEnabled(false);
    
    mDepthViewport->setVisibilityMask(~DEPTHMAP_DISABLED);

    //re-set texture "DoF_Depth"
    MaterialPtr p = MaterialManager::getSingleton().getByName("ssao");
    p->load();
   
    p->getBestTechnique()->getPass(0)->getTextureUnitState("depth")->setTextureName("DoF_Depth");
    p->unload();
}

void SSAOManager::destroyDepthRenderTexture()
{
    mDepthViewport->setRenderQueueInvocationSequenceName("");
    Root::getSingleton().destroyRenderQueueInvocationSequence("DoF_Depth");

    mDepthMaterial->unload();

    mrt->removeAllListeners();
    mrt->removeAllViewports();
    //TextureManager::getSingleton().unload("DoF_Depth");
    TextureManager::getSingleton().remove("DoF_Depth");
}

void SSAOManager::notifyMaterialSetup(uint32 passId, MaterialPtr& material)
{
    Ogre::CompositorInstance::Listener::notifyMaterialSetup(passId, material);
}

void SSAOManager::postViewportUpdate(const RenderTargetViewportEvent& evt)
{
    // Reset the RenderableListener
    RenderQueue* queue = evt.source->getCamera()->getSceneManager()->getRenderQueue();
    queue->setRenderableListener(0);
}

void SSAOManager::preViewportUpdate(const RenderTargetViewportEvent& evt)
{
    float dofParams[4] = {0, 50, 100, 200};

    // Adjust fragment program parameters for depth pass
    
    GpuProgramParametersSharedPtr fragParams =
        mDepthTechnique->getPass(0)->getFragmentProgramParameters();
    if ((!fragParams.isNull())&&(fragParams->_findNamedConstantDefinition("dofParams")))
        fragParams->setNamedConstant("dofParams", dofParams,1,4);       

    evt.source->setVisibilityMask(~DEPTHMAP_DISABLED);

    // Add 'this' as a RenderableListener to replace the technique for all renderables
    RenderQueue* queue = evt.source->getCamera()->getSceneManager()->getRenderQueue();
    queue->setRenderableListener(this);
}

void SSAOManager::removeCompositor()
{

}

bool SSAOManager::renderableQueued(Renderable* rend, uint8 groupID, Ogre::ushort priority, Technique** ppTech, RenderQueue* pQueue)
{
    // Replace the technique of all renderables

    std::string s1 = "OgreTerrain";
  
    if ( !rend->getMaterial().isNull() && rend->getMaterial()->getName().substr(0,s1.length()) == s1 )
    {

       return false;
        
    }
    *ppTech = mDepthTechnique;
    return true;
}
