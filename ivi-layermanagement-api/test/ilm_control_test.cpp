/***************************************************************************
 *
 * Copyright 2010-2014 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
 * Copyright (C) 2016 Advanced Driver Information Technology Joint Venture GmbH
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#include <gtest/gtest.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>

#include "TestBase.h"

extern "C" {
    #include "ilm_control.h"
}

class IlmCommandTest : public TestBase, public ::testing::Test {
public:
    void SetUp()
    {
        ASSERT_EQ(ILM_SUCCESS, ilm_initWithNativedisplay((t_ilm_nativedisplay)wlDisplay));

        iviSurfaces.reserve(10);
        struct iviSurface surf;
        for (int i = 0; i < (int)iviSurfaces.capacity(); ++i)
        {
            surf.surface = ivi_application_surface_create(iviApp, i+500, wlSurfaces[i]);
            surf.surface_id = i+500;
            iviSurfaces.push_back(surf);
        }

        wl_display_flush(wlDisplay);
    }

    void TearDown()
    {
        //print_lmc_get_scene();
        t_ilm_layer* layers = NULL;
        t_ilm_int numLayer=0;
        EXPECT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&numLayer, &layers));
        for (t_ilm_int i=0; i<numLayer; i++)
        {
            EXPECT_EQ(ILM_SUCCESS, ilm_layerRemove(layers[i]));
        };
        free(layers);

        for (std::vector<iviSurface>::reverse_iterator it = iviSurfaces.rbegin();
             it != iviSurfaces.rend();
             ++it)
        {
            ivi_surface_destroy((*it).surface);
        }
        iviSurfaces.clear();

        EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());
        EXPECT_EQ(ILM_SUCCESS, ilm_destroy());
    }
};

TEST_F(IlmCommandTest, SetGetSurfaceOpacity) {
    uint surface1 = iviSurfaces[0].surface_id;
    uint surface2 = iviSurfaces[1].surface_id;
    t_ilm_float opacity;

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetOpacity(surface1, 0.88));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceGetOpacity(surface1, &opacity));
    EXPECT_NEAR(0.88, opacity, 0.01);

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetOpacity(surface2, 0.001));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceGetOpacity(surface2, &opacity));
    EXPECT_NEAR(0.001, opacity, 0.01);
}

TEST_F(IlmCommandTest, SetGetSurfaceOpacity_InvalidInput) {
    t_ilm_uint surface = 0xdeadbeef;
    t_ilm_float opacity;

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetOpacity(surface, 0.88));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
    ASSERT_NE(ILM_SUCCESS, ilm_surfaceGetOpacity(surface, &opacity));
}

TEST_F(IlmCommandTest, SetGetLayerOpacity) {
    uint layer1 = 36;
    uint layer2 = 44;
    t_ilm_float opacity;

    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer1, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer2, 800, 480));

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetOpacity(layer1, 0.88));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_layerGetOpacity(layer1, &opacity));
    EXPECT_NEAR(0.88, opacity, 0.01);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetOpacity(layer2, 0.001));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_layerGetOpacity(layer2, &opacity));
    EXPECT_NEAR(0.001, opacity, 0.01);
}

TEST_F(IlmCommandTest, SetGetLayerOpacity_InvalidInput) {
    t_ilm_layer layer = 0xdeadbeef;
    t_ilm_float opacity;

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetOpacity(layer, 0.88));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
    ASSERT_NE(ILM_SUCCESS, ilm_layerGetOpacity(layer, &opacity));
}

TEST_F(IlmCommandTest, SetGetSurfaceVisibility) {
    uint surface = iviSurfaces[0].surface_id;
    t_ilm_bool visibility;

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetVisibility(surface, ILM_TRUE));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceGetVisibility(surface, &visibility));
    ASSERT_EQ(ILM_TRUE, visibility);

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetVisibility(surface, ILM_FALSE));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceGetVisibility(surface, &visibility));
    ASSERT_EQ(ILM_FALSE, visibility);

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetVisibility(surface, ILM_TRUE));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceGetVisibility(surface, &visibility));
    ASSERT_EQ(ILM_TRUE, visibility);
}

TEST_F(IlmCommandTest, SetGetSurfaceVisibility_InvalidInput) {
    uint surface = 0xdeadbeef;
    t_ilm_bool visibility;

    ASSERT_NE(ILM_SUCCESS, ilm_surfaceGetVisibility(surface, &visibility));
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetVisibility(surface, ILM_TRUE));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetVisibility(surface, ILM_FALSE));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
}

TEST_F(IlmCommandTest, SetGetLayerVisibility) {
    uint layer = 36;
    t_ilm_bool visibility;

    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer, 800, 480));

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetVisibility(layer, ILM_TRUE));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_layerGetVisibility(layer, &visibility));
    ASSERT_EQ(ILM_TRUE, visibility);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetVisibility(layer, ILM_FALSE));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_layerGetVisibility(layer, &visibility));
    ASSERT_EQ(ILM_FALSE, visibility);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetVisibility(layer, ILM_TRUE));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_layerGetVisibility(layer, &visibility));
    ASSERT_EQ(ILM_TRUE, visibility);
}

TEST_F(IlmCommandTest, SetGetLayerVisibility_InvalidInput) {
    uint layer = 0xdeadbeef;
    t_ilm_bool visibility;

    ASSERT_NE(ILM_SUCCESS, ilm_layerGetVisibility(layer, &visibility));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetVisibility(layer, ILM_TRUE));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetVisibility(layer, ILM_FALSE));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
}

TEST_F(IlmCommandTest, SetSurfaceSourceRectangle) {
    t_ilm_uint surface = iviSurfaces[0].surface_id;

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetSourceRectangle(surface, 89, 6538, 638, 4));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    ilmSurfaceProperties surfaceProperties;
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfSurface(surface, &surfaceProperties));
    ASSERT_EQ(89u, surfaceProperties.sourceX);
    ASSERT_EQ(6538u, surfaceProperties.sourceY);
    ASSERT_EQ(638u, surfaceProperties.sourceWidth);
    ASSERT_EQ(4u, surfaceProperties.sourceHeight);
}

TEST_F(IlmCommandTest, SetSurfaceSourceRectangle_InvalidInput) {
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetSourceRectangle(0xdeadbeef, 89, 6538, 638, 4));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
}

TEST_F(IlmCommandTest, ilm_getScreenIDs) {
    t_ilm_uint numberOfScreens = 0;
    t_ilm_uint* screenIDs = NULL;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenIDs(&numberOfScreens, &screenIDs));
    EXPECT_GT(numberOfScreens, 0u);
    free(screenIDs);
}

TEST_F(IlmCommandTest, ilm_getScreenResolution_SingleScreen) {
    t_ilm_uint numberOfScreens = 0;
    t_ilm_uint* screenIDs = NULL;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenIDs(&numberOfScreens, &screenIDs));
    EXPECT_TRUE(numberOfScreens>0);

    if (numberOfScreens > 0)
    {
        uint firstScreen = screenIDs[0];
        t_ilm_uint width = 0, height = 0;
        EXPECT_EQ(ILM_SUCCESS, ilm_getScreenResolution(firstScreen, &width, &height));
        EXPECT_GT(width, 0u);
        EXPECT_GT(height, 0u);
    }

    free(screenIDs);
}

TEST_F(IlmCommandTest, ilm_getScreenResolution_MultiScreen) {
    t_ilm_uint numberOfScreens = 0;
    t_ilm_uint* screenIDs = NULL;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenIDs(&numberOfScreens, &screenIDs));
    EXPECT_TRUE(numberOfScreens>0);

    for (uint screenIndex = 0; screenIndex < numberOfScreens; ++screenIndex)
    {
        uint screen = screenIDs[screenIndex];
        t_ilm_uint width = 0, height = 0;
        EXPECT_EQ(ILM_SUCCESS, ilm_getScreenResolution(screen, &width, &height));
        EXPECT_GT(width, 0u);
        EXPECT_GT(height, 0u);
    }

    free(screenIDs);
}

TEST_F(IlmCommandTest, ilm_getLayerIDs) {
    uint layer1 = 3246;
    uint layer2 = 46586;

    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer1, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer2, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    t_ilm_int length;
    t_ilm_uint* IDs;
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&length, &IDs));

    EXPECT_EQ(layer1, IDs[0]);
    EXPECT_EQ(layer2, IDs[1]);
    free(IDs);
}

TEST_F(IlmCommandTest, ilm_getLayerIDsOfScreen) {
    t_ilm_layer layer1 = 3246;
    t_ilm_layer layer2 = 46586;
    t_ilm_uint roLength = 2;
    t_ilm_layer idRenderOrder[2] = {layer1, layer2};
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer1, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer2, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_displaySetRenderOrder(0, idRenderOrder, roLength));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    t_ilm_int length = 0;
    t_ilm_layer* IDs = 0;
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDsOnScreen(0, &length, &IDs));

    EXPECT_EQ(2, length);
    if (length == 2)
    {
        EXPECT_EQ(layer1, IDs[0]);
        EXPECT_EQ(layer2, IDs[1]);
    }
    free(IDs);
}

TEST_F(IlmCommandTest, ilm_getSurfaceIDs) {
    t_ilm_uint* IDs;
    t_ilm_int length;

    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDs(&length, &IDs));

    EXPECT_EQ(iviSurfaces.size(), length);
    for (int i=0; i < iviSurfaces.size(); i++)
    {
        EXPECT_EQ(IDs[i], iviSurfaces[i].surface_id);
    }
    free(IDs);
}

TEST_F(IlmCommandTest, ilm_layerCreate_Remove) {
    uint layer1 = 3246;
    uint layer2 = 46586;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer1, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer2, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    t_ilm_int length;
    t_ilm_uint* IDs;
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&length, &IDs));

    EXPECT_EQ(length, 2);
    if (length == 2)
    {
        EXPECT_EQ(layer1, IDs[0]);
        EXPECT_EQ(layer2, IDs[1]);
    }
    free(IDs);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerRemove(layer1));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&length, &IDs));
    EXPECT_EQ(length, 1);
    if (length == 1)
    {
        EXPECT_EQ(layer2, IDs[0]);
    }
    free(IDs);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerRemove(layer2));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&length, &IDs));
    EXPECT_EQ(length, 0);
    free(IDs);
}

TEST_F(IlmCommandTest, ilm_layerRemove_InvalidInput) {
    ASSERT_EQ(ILM_SUCCESS, ilm_layerRemove(0xdeadbeef));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
}

TEST_F(IlmCommandTest, ilm_layerRemove_InvalidUse) {
    uint layer = 0xbeef;
    t_ilm_uint* IDs;
    t_ilm_int orig_length;
    t_ilm_int length;

    // get the initial number of layers
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&orig_length, &IDs));
    free(IDs);

    // add the layer
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&length, &IDs));
    free(IDs);
    ASSERT_EQ(length, orig_length+1);

    // remove the new layer
    ASSERT_EQ(ILM_SUCCESS, ilm_layerRemove(layer));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getLayerIDs(&length, &IDs));
    free(IDs);
    ASSERT_EQ(length, orig_length);

    // try to remove the same layer once more
    ASSERT_EQ(ILM_SUCCESS, ilm_layerRemove(layer));
    ASSERT_EQ(ILM_ERROR_RESOURCE_NOT_FOUND, ilm_getError());
}

TEST_F(IlmCommandTest, ilm_layerAddSurface_ilm_layerRemoveSurface_ilm_getSurfaceIDsOnLayer) {
    uint layer = 3246;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer, 800, 480));
    uint surface1 = iviSurfaces[0].surface_id;
    uint surface2 = iviSurfaces[1].surface_id;

    t_ilm_int length;
    t_ilm_uint* IDs;
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &length, &IDs));
    free(IDs);
    ASSERT_EQ(length, 0);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerAddSurface(layer, surface1));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &length, &IDs));
    EXPECT_EQ(length, 1);
    if (length == 1)
    {
        EXPECT_EQ(surface1, IDs[0]);
    }
    free(IDs);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerAddSurface(layer, surface2));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &length, &IDs));
    EXPECT_EQ(length, 2);
    if (length == 2)
    {
       EXPECT_EQ(surface1, IDs[0]);
       EXPECT_EQ(surface2, IDs[1]);
    }
    free(IDs);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerRemoveSurface(layer, surface1));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &length, &IDs));
    EXPECT_EQ(length, 1);
    if (length == 1)
    {
        EXPECT_EQ(surface2, IDs[0]);
    }
    free(IDs);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerRemoveSurface(layer, surface2));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &length, &IDs));
    free(IDs);
    ASSERT_EQ(length, 0);
}

TEST_F(IlmCommandTest, ilm_getSurfaceIDsOnLayer_InvalidInput) {
    uint layer = 0xdeadbeef;

    t_ilm_int length;
    t_ilm_uint* IDs;
    ASSERT_NE(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &length, &IDs));
}

TEST_F(IlmCommandTest, ilm_getSurfaceIDsOnLayer_InvalidResources) {
    uint layer = 3246;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer, 800, 480));
    uint surface1 = iviSurfaces[0].surface_id;
    uint surface2 = iviSurfaces[1].surface_id;
    uint surface3 = iviSurfaces[2].surface_id;

    t_ilm_int length;
    t_ilm_uint IDs[3];
    IDs[0] = surface1;
    IDs[1] = surface2;
    IDs[2] = surface3;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, IDs, 3));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    ASSERT_NE(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &length, NULL));
}

TEST_F(IlmCommandTest, ilm_getPropertiesOfSurface_ilm_surfaceSetSourceRectangle_ilm_surfaceSetDestinationRectangle) {
    uint surface = iviSurfaces[0].surface_id;

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetOpacity(surface, 0.8765));
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetSourceRectangle(surface, 89, 6538, 638, 4));
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetDestinationRectangle(surface, 54, 47, 947, 9));
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetVisibility(surface, true));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    ilmSurfaceProperties surfaceProperties;
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfSurface(surface, &surfaceProperties));
    ASSERT_NEAR(0.8765, surfaceProperties.opacity, 0.1);
    ASSERT_EQ(89u, surfaceProperties.sourceX);
    ASSERT_EQ(6538u, surfaceProperties.sourceY);
    ASSERT_EQ(638u, surfaceProperties.sourceWidth);
    ASSERT_EQ(4u, surfaceProperties.sourceHeight);
    ASSERT_EQ(54u, surfaceProperties.destX);
    ASSERT_EQ(47u, surfaceProperties.destY);
    ASSERT_EQ(947u, surfaceProperties.destWidth);
    ASSERT_EQ(9u, surfaceProperties.destHeight);
    ASSERT_TRUE( surfaceProperties.visibility);
    ASSERT_EQ(getpid(), surfaceProperties.creatorPid);

    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetOpacity(surface, 0.436));
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetSourceRectangle(surface, 784, 546, 235, 78));
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetDestinationRectangle(surface, 536, 5372, 3, 4316));
    ASSERT_EQ(ILM_SUCCESS, ilm_surfaceSetVisibility(surface, false));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    ilmSurfaceProperties surfaceProperties2;
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfSurface(surface, &surfaceProperties2));
    ASSERT_NEAR(0.436, surfaceProperties2.opacity, 0.1);
    ASSERT_EQ(784u, surfaceProperties2.sourceX);
    ASSERT_EQ(546u, surfaceProperties2.sourceY);
    ASSERT_EQ(235u, surfaceProperties2.sourceWidth);
    ASSERT_EQ(78u, surfaceProperties2.sourceHeight);
    ASSERT_EQ(536u, surfaceProperties2.destX);
    ASSERT_EQ(5372u, surfaceProperties2.destY);
    ASSERT_EQ(3u, surfaceProperties2.destWidth);
    ASSERT_EQ(4316u, surfaceProperties2.destHeight);
    ASSERT_FALSE(surfaceProperties2.visibility);
}

TEST_F(IlmCommandTest, ilm_getPropertiesOfLayer_ilm_layerSetSourceRectangle_ilm_layerSetDestinationRectangle) {
    t_ilm_uint layer = 0xbeef;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer, 800, 480));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetOpacity(layer, 0.8765));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetSourceRectangle(layer, 89, 6538, 638, 4));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetDestinationRectangle(layer, 54, 47, 947, 9));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetVisibility(layer, true));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    ilmLayerProperties layerProperties1;
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfLayer(layer, &layerProperties1));
    ASSERT_NEAR(0.8765, layerProperties1.opacity, 0.1);
    ASSERT_EQ(89u, layerProperties1.sourceX);
    ASSERT_EQ(6538u, layerProperties1.sourceY);
    ASSERT_EQ(638u, layerProperties1.sourceWidth);
    ASSERT_EQ(4u, layerProperties1.sourceHeight);
    ASSERT_EQ(54u, layerProperties1.destX);
    ASSERT_EQ(47u, layerProperties1.destY);
    ASSERT_EQ(947u, layerProperties1.destWidth);
    ASSERT_EQ(9u, layerProperties1.destHeight);
    ASSERT_TRUE( layerProperties1.visibility);

    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetOpacity(layer, 0.436));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetSourceRectangle(layer, 784, 546, 235, 78));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetDestinationRectangle(layer, 536, 5372, 3, 4316));
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetVisibility(layer, false));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    ilmLayerProperties layerProperties2;
    ASSERT_EQ(ILM_SUCCESS, ilm_getPropertiesOfLayer(layer, &layerProperties2));
    ASSERT_NEAR(0.436, layerProperties2.opacity, 0.1);
    ASSERT_EQ(784u, layerProperties2.sourceX);
    ASSERT_EQ(546u, layerProperties2.sourceY);
    ASSERT_EQ(235u, layerProperties2.sourceWidth);
    ASSERT_EQ(78u, layerProperties2.sourceHeight);
    ASSERT_EQ(536u, layerProperties2.destX);
    ASSERT_EQ(5372u, layerProperties2.destY);
    ASSERT_EQ(3u, layerProperties2.destWidth);
    ASSERT_EQ(4316u, layerProperties2.destHeight);
    ASSERT_FALSE(layerProperties2.visibility);
}

TEST_F(IlmCommandTest, ilm_getPropertiesOfSurface_InvalidInput) {
    ilmSurfaceProperties surfaceProperties;
    ASSERT_NE(ILM_SUCCESS, ilm_getPropertiesOfSurface(0xdeadbeef, &surfaceProperties));
}

TEST_F(IlmCommandTest, ilm_takeScreenshot) {
    const char* outputFile = "/tmp/test.bmp";
    // make sure the file is not there before
    FILE* f = fopen(outputFile, "r");
    if (f!=NULL){
        fclose(f);
        int result = remove(outputFile);
        ASSERT_EQ(0, result);
    }

    ASSERT_EQ(ILM_SUCCESS, ilm_takeScreenshot(0, outputFile));

    f = fopen(outputFile, "r");
    ASSERT_TRUE(f!=NULL);
    fclose(f);
    remove(outputFile);
}

TEST_F(IlmCommandTest, ilm_takeScreenshot_InvalidInputs) {
    const char* outputFile = "/tmp/test.bmp";
    // make sure the file is not there before
    FILE* f = fopen(outputFile, "r");
    if (f!=NULL){
        fclose(f);
        ASSERT_EQ(0, remove(outputFile));
    }

    // try to dump an non-existing screen
    ASSERT_NE(ILM_SUCCESS, ilm_takeScreenshot(0xdeadbeef, outputFile));

    // make sure, no screen dump file was created for invalid screen
    ASSERT_NE(0, remove(outputFile));
}

TEST_F(IlmCommandTest, ilm_takeSurfaceScreenshot) {
    const char* outputFile = "/tmp/test.bmp";
    // make sure the file is not there before
    FILE* f = fopen(outputFile, "r");
    if (f!=NULL){
        fclose(f);
        int result = remove(outputFile);
        ASSERT_EQ(0, result);
    }

    uint surface = iviSurfaces[0].surface_id;
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_takeSurfaceScreenshot(outputFile, surface));

    f = fopen(outputFile, "r");
    ASSERT_TRUE(f!=NULL);
    fclose(f);
    remove(outputFile);
}

TEST_F(IlmCommandTest, ilm_takeSurfaceScreenshot_InvalidInputs) {
    const char* outputFile = "/tmp/test.bmp";
    // make sure the file is not there before
    FILE* f = fopen(outputFile, "r");
    if (f!=NULL){
        fclose(f);
        ASSERT_EQ(0, remove(outputFile));
    }

    // try to dump an non-existing screen
    ASSERT_EQ(ILM_FAILED, ilm_takeSurfaceScreenshot(outputFile, 0xdeadbeef));

    // make sure, no screen dump file was created for invalid screen
    ASSERT_NE(0, remove(outputFile));
}

TEST_F(IlmCommandTest, ilm_getPropertiesOfScreen) {
    t_ilm_uint numberOfScreens = 0;
    t_ilm_uint* screenIDs = NULL;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenIDs(&numberOfScreens, &screenIDs));
    EXPECT_TRUE(numberOfScreens>0);

    if (numberOfScreens > 0)
    {
        t_ilm_display screen = screenIDs[0];
        ilmScreenProperties screenProperties;

        t_ilm_layer layerIds[3] = {100, 200, 300};//t_ilm_layer layerIds[3] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
        EXPECT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(layerIds, 800, 480));
        EXPECT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(layerIds + 1, 800, 480));
        EXPECT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(layerIds + 2, 800, 480));

        EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());

        EXPECT_EQ(ILM_SUCCESS, ilm_displaySetRenderOrder(screen, layerIds, 3));

        EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());


        EXPECT_EQ(ILM_SUCCESS, ilm_getPropertiesOfScreen(screen, &screenProperties));
        EXPECT_EQ(3, screenProperties.layerCount);
        if (screenProperties.layerCount == 3)
        {
            EXPECT_EQ(layerIds[0], screenProperties.layerIds[0]);
            EXPECT_EQ(layerIds[1], screenProperties.layerIds[1]);
            EXPECT_EQ(layerIds[2], screenProperties.layerIds[2]);
        }
        free(screenProperties.layerIds);

        EXPECT_GT(screenProperties.screenWidth, 0u);
        EXPECT_GT(screenProperties.screenHeight, 0u);
    }

    free(screenIDs);
}

TEST_F(IlmCommandTest, DisplaySetRenderOrder_growing) {
    //prepare needed layers
    t_ilm_layer renderOrder[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    t_ilm_uint layerCount = sizeof(renderOrder) / sizeof(renderOrder[0]);

    for (unsigned int i = 0; i < layerCount; ++i)
    {
        ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(renderOrder + i, 300, 300));
        ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    }

    t_ilm_display* screenIDs;
    t_ilm_uint screenCount;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenIDs(&screenCount, &screenIDs));

    for(unsigned int i = 0; i < screenCount; ++i)
    {
        t_ilm_display screen = screenIDs[i];
        ilmScreenProperties screenProps;

        //trying different render orders with increasing sizes
        for (unsigned int j = layerCount; j <= layerCount; --j) // note: using overflow here
        {
            //put them from end to beginning, so that in each loop iteration the order of layers change
            EXPECT_EQ(ILM_SUCCESS, ilm_displaySetRenderOrder(screen, renderOrder + j, layerCount - j));
            EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());
            EXPECT_EQ(ILM_SUCCESS, ilm_getPropertiesOfScreen(screen, &screenProps));

            EXPECT_EQ(layerCount - j, screenProps.layerCount);
            if (layerCount - j == screenProps.layerCount)
                for(unsigned int k = 0; k < layerCount - j; ++k)
                {
                    EXPECT_EQ(renderOrder[j + k], screenProps.layerIds[k]);
                }
            free(screenProps.layerIds);
        }
    }

    free(screenIDs);
}

TEST_F(IlmCommandTest, DisplaySetRenderOrder_shrinking) {
    //prepare needed layers
    t_ilm_layer renderOrder[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    t_ilm_uint layerCount = sizeof(renderOrder) / sizeof(renderOrder[0]);

    for (unsigned int i = 0; i < layerCount; ++i)
    {
        ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(renderOrder + i, 300, 300));
        ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    }

    t_ilm_display* screenIDs;
    t_ilm_uint screenCount;
    ASSERT_EQ(ILM_SUCCESS, ilm_getScreenIDs(&screenCount, &screenIDs));

    for(unsigned int i = 0; i < screenCount; ++i)
    {
        t_ilm_display screen = screenIDs[i];
        ilmScreenProperties screenProps;

        //trying different render orders with decreasing sizes
        for (unsigned int j = 0; j < layerCount; ++j)
        {
            //put them from end to beginning, so that in each loop iteration the order of layers change
            EXPECT_EQ(ILM_SUCCESS, ilm_displaySetRenderOrder(screen, renderOrder + j, layerCount - j));
            EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());
            EXPECT_EQ(ILM_SUCCESS, ilm_getPropertiesOfScreen(screen, &screenProps));

            EXPECT_EQ(layerCount - j, screenProps.layerCount);
            if (layerCount - j == screenProps.layerCount)
                for(unsigned int k = 0; k < layerCount - j; ++k)
                {
                    ASSERT_EQ(renderOrder[j + k], screenProps.layerIds[k]);
                }
            free(screenProps.layerIds);
        }
    }

    free(screenIDs);
}

TEST_F(IlmCommandTest, LayerSetRenderOrder_growing) {
    //prepare needed layers and surfaces
    unsigned int renderOrder[] = {0, 0, 0};
    t_ilm_uint surfaceCount = 3;

    for (unsigned int i = 0; i < surfaceCount; ++i)
    {
        renderOrder[i] = iviSurfaces[i].surface_id;
    }

    t_ilm_layer layerIDs[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    t_ilm_uint layerCount = sizeof(layerIDs) / sizeof(layerIDs[0]);

    for (unsigned int i = 0; i < layerCount; ++i)
    {
        ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(layerIDs + i, 300, 300));
        ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    }

    for(unsigned int i = 0; i < layerCount; ++i)
    {
        t_ilm_layer layer = layerIDs[i];

        t_ilm_int layerSurfaceCount;
        t_ilm_surface* layerSurfaceIDs;

        //trying different render orders with increasing sizes
        for (unsigned int j = surfaceCount; j <= surfaceCount; --j) // note: using overflow here
        {
            //put them from end to beginning, so that in each loop iteration the order of surafces change
            EXPECT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, renderOrder + j, surfaceCount - j));
            EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());
            EXPECT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &layerSurfaceCount, &layerSurfaceIDs));

            EXPECT_EQ(surfaceCount - j, layerSurfaceCount);
            if (surfaceCount - j == (unsigned int) layerSurfaceCount)
                for(unsigned int k = 0; k < surfaceCount - j; ++k)
                {
                    ASSERT_EQ(renderOrder[j + k], layerSurfaceIDs[k]);
                }
            free(layerSurfaceIDs);
        }

        //set empty render order again
        EXPECT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, renderOrder, 0));
        EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());
    }
}

TEST_F(IlmCommandTest, LayerSetRenderOrder_shrinking) {
    //prepare needed layers and surfaces
    unsigned int renderOrder[] = {0, 0, 0};
    t_ilm_uint surfaceCount = 3;

    for (unsigned int i = 0; i < surfaceCount; ++i)
    {
        renderOrder[i] = iviSurfaces[i].surface_id;
    }

    t_ilm_layer layerIDs[] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};
    t_ilm_uint layerCount = sizeof(layerIDs) / sizeof(layerIDs[0]);

    for (unsigned int i = 0; i < layerCount; ++i)
    {
        ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(layerIDs + i, 300, 300));
        ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    }

    for(unsigned int i = 0; i < layerCount; ++i)
    {
        t_ilm_layer layer = layerIDs[i];

        t_ilm_int layerSurfaceCount;
        t_ilm_surface* layerSurfaceIDs;

        //trying different render orders with decreasing sizes
        for (unsigned int j = 0; j < layerCount; ++j)
        {
            //put them from end to beginning, so that in each loop iteration the order of surafces change
            EXPECT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, renderOrder + j, surfaceCount - j));
            EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());
            EXPECT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &layerSurfaceCount, &layerSurfaceIDs));

            EXPECT_EQ(surfaceCount - j, layerSurfaceCount);
            if (surfaceCount - j == (unsigned int)layerSurfaceCount)
                for(unsigned int k = 0; k < surfaceCount - j; ++k)
                {
                    EXPECT_EQ(renderOrder[j + k], layerSurfaceIDs[k]);
                }
            free(layerSurfaceIDs);
        }

        //set empty render order again
        EXPECT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, renderOrder, 0));
        EXPECT_EQ(ILM_SUCCESS, ilm_commitChanges());
    }
}

TEST_F(IlmCommandTest, LayerSetRenderOrder_duplicates) {
    //prepare needed layers and surfaces
    unsigned int renderOrder[] = {0, 0, 0};
    t_ilm_uint surfaceCount = 3;

    for (unsigned int i = 0; i < surfaceCount; ++i)
    {
        renderOrder[i] = iviSurfaces[i].surface_id;
    }

    t_ilm_surface duplicateRenderOrder[] = {renderOrder[0], renderOrder[1], renderOrder[0], renderOrder[1], renderOrder[0]};
    t_ilm_int duplicateSurfaceCount = sizeof(duplicateRenderOrder) / sizeof(duplicateRenderOrder[0]);

    t_ilm_layer layer = 0xFFFFFFFF;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer, 300, 300));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    t_ilm_int layerSurfaceCount;
    t_ilm_surface* layerSurfaceIDs;

    //trying duplicates
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, duplicateRenderOrder, duplicateSurfaceCount));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &layerSurfaceCount, &layerSurfaceIDs));
    free(layerSurfaceIDs);

    ASSERT_EQ(2, layerSurfaceCount);
}

TEST_F(IlmCommandTest, LayerSetRenderOrder_empty) {
    //prepare needed layers and surfaces
    unsigned int renderOrder[] = {0, 0, 0};
    t_ilm_uint surfaceCount = 3;

    for (unsigned int i = 0; i < surfaceCount; ++i)
    {
        renderOrder[i] = iviSurfaces[i].surface_id;
    }

    t_ilm_layer layer = 0xFFFFFFFF;
    ASSERT_EQ(ILM_SUCCESS, ilm_layerCreateWithDimension(&layer, 300, 300));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    t_ilm_int layerSurfaceCount;
    t_ilm_surface* layerSurfaceIDs;

    //test start
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, renderOrder, surfaceCount));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());

    //set empty render order
    ASSERT_EQ(ILM_SUCCESS, ilm_layerSetRenderOrder(layer, renderOrder, 0));
    ASSERT_EQ(ILM_SUCCESS, ilm_commitChanges());
    ASSERT_EQ(ILM_SUCCESS, ilm_getSurfaceIDsOnLayer(layer, &layerSurfaceCount, &layerSurfaceIDs));
    free(layerSurfaceIDs);

    ASSERT_EQ(0, layerSurfaceCount);
}
