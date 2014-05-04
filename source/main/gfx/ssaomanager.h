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



#ifndef SSAOMANAGER_H
#define SSAOMANAGER_H
#include "Ogre.h"
class SSAOManager : public Ogre::CompositorInstance::Listener,
                           public Ogre::RenderTargetListener,
                           public Ogre::RenderQueue::RenderableListener
{
public:
    SSAOManager();
    ~SSAOManager();
    void loadConfiguration();
private:
        // Implementation of Ogre::CompositorInstance::Listener
    virtual void notifyMaterialSetup(Ogre::uint32 passId, Ogre::MaterialPtr& material);

    // Implementation of Ogre::RenderTargetListener
    virtual void preViewportUpdate(const Ogre::RenderTargetViewportEvent& evt);
    virtual void postViewportUpdate(const Ogre::RenderTargetViewportEvent& evt);

    // Implementation of Ogre::RenderQueue::RenderableListener
    virtual bool renderableQueued(Ogre::Renderable* rend, Ogre::uint8 groupID,
                Ogre::ushort priority, Ogre::Technique** ppTech, Ogre::RenderQueue* pQueue);
    
    
    Ogre::Viewport* mDepthViewport;
    Ogre::RenderTexture* mDepthTarget;
    Ogre::TexturePtr mDepthTexture;
    Ogre::TexturePtr mPositionTexture;
    Ogre::MaterialPtr mDepthMaterial;
    Ogre::Technique* mDepthTechnique;
    Ogre::CompositorInstance* mCompositor;
    Ogre::MultiRenderTarget* mrt;
    int mWidth;
    int mHeight;
    Ogre::MaterialPtr mTerrainDepthMaterial;
    Ogre::Technique* mTerrainTech;


    void createDepthRenderTexture();
    void destroyDepthRenderTexture();
//  void createCompositor();
//  void destroyCompositor();
    void addCompositor();
    void removeCompositor();

};

#endif // SSAOMANAGER_H
