// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "config.h"

#include "cc/layer_tree_host_common.h"

#include "cc/content_layer.h"
#include "cc/content_layer_client.h"
#include "cc/layer.h"
#include "cc/layer_animation_controller.h"
#include "cc/layer_impl.h"
#include "cc/layer_sorter.h"
#include "cc/math_util.h"
#include "cc/proxy.h"
#include "cc/single_thread_proxy.h"
#include "cc/test/animation_test_common.h"
#include "cc/test/geometry_test_utils.h"
#include "cc/thread.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include <public/WebTransformationMatrix.h>

using namespace cc;
using namespace WebKitTests;
using WebKit::WebTransformationMatrix;

namespace {

template<typename LayerType>
void setLayerPropertiesForTestingInternal(LayerType* layer, const WebTransformationMatrix& transform, const WebTransformationMatrix& sublayerTransform, const gfx::PointF& anchor, const gfx::PointF& position, const gfx::Size& bounds, bool preserves3D)
{
    layer->setTransform(transform);
    layer->setSublayerTransform(sublayerTransform);
    layer->setAnchorPoint(anchor);
    layer->setPosition(position);
    layer->setBounds(bounds);
    layer->setPreserves3D(preserves3D);
}

void setLayerPropertiesForTesting(Layer* layer, const WebTransformationMatrix& transform, const WebTransformationMatrix& sublayerTransform, const gfx::PointF& anchor, const gfx::PointF& position, const gfx::Size& bounds, bool preserves3D)
{
    setLayerPropertiesForTestingInternal<Layer>(layer, transform, sublayerTransform, anchor, position, bounds, preserves3D);
    layer->setAutomaticallyComputeRasterScale(true);
}

void setLayerPropertiesForTesting(LayerImpl* layer, const WebTransformationMatrix& transform, const WebTransformationMatrix& sublayerTransform, const gfx::PointF& anchor, const gfx::PointF& position, const gfx::Size& bounds, bool preserves3D)
{
    setLayerPropertiesForTestingInternal<LayerImpl>(layer, transform, sublayerTransform, anchor, position, bounds, preserves3D);
    layer->setContentBounds(bounds);
}

void executeCalculateDrawTransformsAndVisibility(Layer* rootLayer, float deviceScaleFactor = 1, float pageScaleFactor = 1)
{
    WebTransformationMatrix identityMatrix;
    std::vector<scoped_refptr<Layer> > dummyRenderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    gfx::Size deviceViewportSize = gfx::Size(rootLayer->bounds().width() * deviceScaleFactor, rootLayer->bounds().height() * deviceScaleFactor);

    // We are probably not testing what is intended if the rootLayer bounds are empty.
    DCHECK(!rootLayer->bounds().IsEmpty());
    LayerTreeHostCommon::calculateDrawTransforms(rootLayer, deviceViewportSize, deviceScaleFactor, pageScaleFactor, dummyMaxTextureSize, dummyRenderSurfaceLayerList);
}

void executeCalculateDrawTransformsAndVisibility(LayerImpl* rootLayer, float deviceScaleFactor = 1, float pageScaleFactor = 1)
{
    // Note: this version skips layer sorting.

    WebTransformationMatrix identityMatrix;
    std::vector<LayerImpl*> dummyRenderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    gfx::Size deviceViewportSize = gfx::Size(rootLayer->bounds().width() * deviceScaleFactor, rootLayer->bounds().height() * deviceScaleFactor);

    // We are probably not testing what is intended if the rootLayer bounds are empty.
    DCHECK(!rootLayer->bounds().IsEmpty());
    LayerTreeHostCommon::calculateDrawTransforms(rootLayer, deviceViewportSize, deviceScaleFactor, pageScaleFactor, 0, dummyMaxTextureSize, dummyRenderSurfaceLayerList);
}

WebTransformationMatrix remove3DComponentOfMatrix(const WebTransformationMatrix& mat)
{
    WebTransformationMatrix ret = mat;
    ret.setM13(0);
    ret.setM23(0);
    ret.setM31(0);
    ret.setM32(0);
    ret.setM33(1);
    ret.setM34(0);
    ret.setM43(0);
    return ret;
}

scoped_ptr<LayerImpl> createTreeForFixedPositionTests()
{
    scoped_ptr<LayerImpl> root = LayerImpl::create(1);
    scoped_ptr<LayerImpl> child = LayerImpl::create(2);
    scoped_ptr<LayerImpl> grandChild = LayerImpl::create(3);
    scoped_ptr<LayerImpl> greatGrandChild = LayerImpl::create(4);

    WebTransformationMatrix IdentityMatrix;
    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), IdentityMatrix, IdentityMatrix, anchor, position, bounds, false);
    setLayerPropertiesForTesting(child.get(), IdentityMatrix, IdentityMatrix, anchor, position, bounds, false);
    setLayerPropertiesForTesting(grandChild.get(), IdentityMatrix, IdentityMatrix, anchor, position, bounds, false);
    setLayerPropertiesForTesting(greatGrandChild.get(), IdentityMatrix, IdentityMatrix, anchor, position, bounds, false);

    grandChild->addChild(greatGrandChild.Pass());
    child->addChild(grandChild.Pass());
    root->addChild(child.Pass());

    return root.Pass();
}

class LayerWithForcedDrawsContent : public Layer {
public:
    LayerWithForcedDrawsContent()
        : Layer()
    {
    }

    virtual bool drawsContent() const OVERRIDE { return true; }

private:
    virtual ~LayerWithForcedDrawsContent()
    {
    }
};

class MockContentLayerClient : public ContentLayerClient {
public:
    MockContentLayerClient() { }
    virtual ~MockContentLayerClient() { }
    virtual void paintContents(SkCanvas*, const gfx::Rect& clip, gfx::RectF& opaque) OVERRIDE { }
};

scoped_refptr<ContentLayer> createDrawableContentLayer(ContentLayerClient* delegate)
{
    scoped_refptr<ContentLayer> toReturn = ContentLayer::create(delegate);
    toReturn->setIsDrawable(true);
    return toReturn;
}

#define EXPECT_CONTENTS_SCALE_EQ(expected, layer) \
  do { \
    EXPECT_FLOAT_EQ(expected, layer->contentsScaleX()); \
    EXPECT_FLOAT_EQ(expected, layer->contentsScaleY()); \
  } while (false)

TEST(LayerTreeHostCommonTest, verifyTransformsForNoOpLayer)
{
    // Sanity check: For layers positioned at zero, with zero size,
    // and with identity transforms, then the drawTransform,
    // screenSpaceTransform, and the hierarchy passed on to children
    // layers should also be identity transforms.

    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> grandChild = Layer::create();
    parent->addChild(child);
    child->addChild(grandChild);

    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(0, 0), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(0, 0), false);

    executeCalculateDrawTransformsAndVisibility(parent.get());

    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, grandChild->screenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyTransformsForSingleLayer)
{
    WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> layer = Layer::create();

    scoped_refptr<Layer> root = Layer::create();
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(1, 2), false);
    root->addChild(layer);

    // Case 1: setting the sublayer transform should not affect this layer's draw transform or screen-space transform.
    WebTransformationMatrix arbitraryTranslation;
    arbitraryTranslation.translate(10, 20);
    setLayerPropertiesForTesting(layer.get(), identityMatrix, arbitraryTranslation, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    WebTransformationMatrix expectedDrawTransform = identityMatrix;
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedDrawTransform, layer->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, layer->screenSpaceTransform());

    // Case 2: Setting the bounds of the layer should not affect either the draw transform or the screenspace transform.
    WebTransformationMatrix translationToCenter;
    translationToCenter.translate(5, 6);
    setLayerPropertiesForTesting(layer.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, layer->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, layer->screenSpaceTransform());

    // Case 3: The anchor point by itself (without a layer transform) should have no effect on the transforms.
    setLayerPropertiesForTesting(layer.get(), identityMatrix, identityMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, layer->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, layer->screenSpaceTransform());

    // Case 4: A change in actual position affects both the draw transform and screen space transform.
    WebTransformationMatrix positionTransform;
    positionTransform.translate(0, 1.2);
    setLayerPropertiesForTesting(layer.get(), identityMatrix, identityMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 1.2f), gfx::Size(10, 12), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(positionTransform, layer->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(positionTransform, layer->screenSpaceTransform());

    // Case 5: In the correct sequence of transforms, the layer transform should pre-multiply the translationToCenter. This is easily tested by
    //         using a scale transform, because scale and translation are not commutative.
    WebTransformationMatrix layerTransform;
    layerTransform.scale3d(2, 2, 1);
    setLayerPropertiesForTesting(layer.get(), layerTransform, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(layerTransform, layer->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(layerTransform, layer->screenSpaceTransform());

    // Case 6: The layer transform should occur with respect to the anchor point.
    WebTransformationMatrix translationToAnchor;
    translationToAnchor.translate(5, 0);
    WebTransformationMatrix expectedResult = translationToAnchor * layerTransform * translationToAnchor.inverse();
    setLayerPropertiesForTesting(layer.get(), layerTransform, identityMatrix, gfx::PointF(0.5, 0), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedResult, layer->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedResult, layer->screenSpaceTransform());

    // Case 7: Verify that position pre-multiplies the layer transform.
    //         The current implementation of calculateDrawTransforms does this implicitly, but it is
    //         still worth testing to detect accidental regressions.
    expectedResult = positionTransform * translationToAnchor * layerTransform * translationToAnchor.inverse();
    setLayerPropertiesForTesting(layer.get(), layerTransform, identityMatrix, gfx::PointF(0.5, 0), gfx::PointF(0, 1.2f), gfx::Size(10, 12), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedResult, layer->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedResult, layer->screenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyTransformsForSimpleHierarchy)
{
    WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> grandChild = Layer::create();
    root->addChild(parent);
    parent->addChild(child);
    child->addChild(grandChild);

    // One-time setup of root layer
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(1, 2), false);

    // Case 1: parent's anchorPoint should not affect child or grandChild.
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(16, 18), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(76, 78), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, grandChild->screenSpaceTransform());

    // Case 2: parent's position affects child and grandChild.
    WebTransformationMatrix parentPositionTransform;
    parentPositionTransform.translate(0, 1.2);
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 1.2f), gfx::Size(10, 12), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(16, 18), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(76, 78), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentPositionTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentPositionTransform, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentPositionTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentPositionTransform, grandChild->screenSpaceTransform());

    // Case 3: parent's local transform affects child and grandchild
    WebTransformationMatrix parentLayerTransform;
    parentLayerTransform.scale3d(2, 2, 1);
    WebTransformationMatrix parentTranslationToAnchor;
    parentTranslationToAnchor.translate(2.5, 3);
    WebTransformationMatrix parentCompositeTransform = parentTranslationToAnchor * parentLayerTransform * parentTranslationToAnchor.inverse();
    setLayerPropertiesForTesting(parent.get(), parentLayerTransform, identityMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(16, 18), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(76, 78), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, grandChild->screenSpaceTransform());

    // Case 4: parent's sublayerMatrix affects child and grandchild
    //         scaling is used here again so that the correct sequence of transforms is properly tested.
    //         Note that preserves3D is false, but the sublayer matrix should retain its 3D properties when given to child.
    //         But then, the child also does not preserve3D. When it gives its hierarchy to the grandChild, it should be flattened to 2D.
    WebTransformationMatrix parentSublayerMatrix;
    parentSublayerMatrix.scale3d(10, 10, 3.3);
    WebTransformationMatrix parentTranslationToCenter;
    parentTranslationToCenter.translate(5, 6);
    // Sublayer matrix is applied to the center of the parent layer.
    parentCompositeTransform = parentTranslationToAnchor * parentLayerTransform * parentTranslationToAnchor.inverse()
            * parentTranslationToCenter * parentSublayerMatrix * parentTranslationToCenter.inverse();
    WebTransformationMatrix flattenedCompositeTransform = remove3DComponentOfMatrix(parentCompositeTransform);
    setLayerPropertiesForTesting(parent.get(), parentLayerTransform, parentSublayerMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(16, 18), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(76, 78), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(flattenedCompositeTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(flattenedCompositeTransform, grandChild->screenSpaceTransform());

    // Case 5: same as Case 4, except that child does preserve 3D, so the grandChild should receive the non-flattened composite transform.
    //
    setLayerPropertiesForTesting(parent.get(), parentLayerTransform, parentSublayerMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(16, 18), true);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(76, 78), false);
    executeCalculateDrawTransformsAndVisibility(root.get());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, grandChild->screenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyTransformsForSingleRenderSurface)
{
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> grandChild = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(parent);
    parent->addChild(child);
    child->addChild(grandChild);

    // One-time setup of root layer
    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(1, 2), false);

    // Child is set up so that a new render surface should be created.
    child->setOpacity(0.5);

    WebTransformationMatrix parentLayerTransform;
    parentLayerTransform.scale3d(1, 0.9, 1);
    WebTransformationMatrix parentTranslationToAnchor;
    parentTranslationToAnchor.translate(25, 30);
    WebTransformationMatrix parentSublayerMatrix;
    parentSublayerMatrix.scale3d(0.9, 1, 3.3);
    WebTransformationMatrix parentTranslationToCenter;
    parentTranslationToCenter.translate(50, 60);
    WebTransformationMatrix parentCompositeTransform = parentTranslationToAnchor * parentLayerTransform * parentTranslationToAnchor.inverse()
            * parentTranslationToCenter * parentSublayerMatrix * parentTranslationToCenter.inverse();
    gfx::Vector2dF parentCompositeScale = MathUtil::computeTransform2dScaleComponents(parentCompositeTransform);
    WebTransformationMatrix surfaceSublayerTransform;
    surfaceSublayerTransform.scaleNonUniform(parentCompositeScale.x(), parentCompositeScale.y());
    WebTransformationMatrix surfaceSublayerCompositeTransform = parentCompositeTransform * surfaceSublayerTransform.inverse();

    // Child's render surface should not exist yet.
    ASSERT_FALSE(child->renderSurface());

    setLayerPropertiesForTesting(parent.get(), parentLayerTransform, parentSublayerMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 0), gfx::Size(100, 120), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(16, 18), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(8, 10), false);
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Render surface should have been created now.
    ASSERT_TRUE(child->renderSurface());
    ASSERT_EQ(child, child->renderTarget());

    // The child layer's draw transform should refer to its new render surface.
    // The screen-space transform, however, should still refer to the root.
    EXPECT_TRANSFORMATION_MATRIX_EQ(surfaceSublayerTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(parentCompositeTransform, child->screenSpaceTransform());

    // Because the grandChild is the only drawable content, the child's renderSurface will tighten its bounds to the grandChild.
    // The scale at which the surface's subtree is drawn must be removed from the composite transform.
    EXPECT_TRANSFORMATION_MATRIX_EQ(surfaceSublayerCompositeTransform, child->renderTarget()->renderSurface()->drawTransform());

    // The screen space is the same as the target since the child surface draws into the root.
    EXPECT_TRANSFORMATION_MATRIX_EQ(surfaceSublayerCompositeTransform, child->renderTarget()->renderSurface()->screenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyTransformsForReplica)
{
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> childReplica = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> grandChild = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(parent);
    parent->addChild(child);
    child->addChild(grandChild);
    child->setReplicaLayer(childReplica.get());

    // One-time setup of root layer
    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(1, 2), false);

    // Child is set up so that a new render surface should be created.
    child->setOpacity(0.5);

    WebTransformationMatrix parentLayerTransform;
    parentLayerTransform.scale3d(2, 2, 1);
    WebTransformationMatrix parentTranslationToAnchor;
    parentTranslationToAnchor.translate(2.5, 3);
    WebTransformationMatrix parentSublayerMatrix;
    parentSublayerMatrix.scale3d(10, 10, 3.3);
    WebTransformationMatrix parentTranslationToCenter;
    parentTranslationToCenter.translate(5, 6);
    WebTransformationMatrix parentCompositeTransform = parentTranslationToAnchor * parentLayerTransform * parentTranslationToAnchor.inverse()
            * parentTranslationToCenter * parentSublayerMatrix * parentTranslationToCenter.inverse();
    WebTransformationMatrix childTranslationToCenter;
    childTranslationToCenter.translate(8, 9);
    WebTransformationMatrix replicaLayerTransform;
    replicaLayerTransform.scale3d(3, 3, 1);
    gfx::Vector2dF parentCompositeScale = MathUtil::computeTransform2dScaleComponents(parentCompositeTransform);
    WebTransformationMatrix surfaceSublayerTransform;
    surfaceSublayerTransform.scaleNonUniform(parentCompositeScale.x(), parentCompositeScale.y());
    WebTransformationMatrix replicaCompositeTransform = parentCompositeTransform * replicaLayerTransform * surfaceSublayerTransform.inverse();

    // Child's render surface should not exist yet.
    ASSERT_FALSE(child->renderSurface());

    setLayerPropertiesForTesting(parent.get(), parentLayerTransform, parentSublayerMatrix, gfx::PointF(0.25, 0.25), gfx::PointF(0, 0), gfx::Size(10, 12), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(16, 18), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(-0.5, -0.5), gfx::Size(1, 1), false);
    setLayerPropertiesForTesting(childReplica.get(), replicaLayerTransform, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(0, 0), false);
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Render surface should have been created now.
    ASSERT_TRUE(child->renderSurface());
    ASSERT_EQ(child, child->renderTarget());

    EXPECT_TRANSFORMATION_MATRIX_EQ(replicaCompositeTransform, child->renderTarget()->renderSurface()->replicaDrawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(replicaCompositeTransform, child->renderTarget()->renderSurface()->replicaScreenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyTransformsForRenderSurfaceHierarchy)
{
    // This test creates a more complex tree and verifies it all at once. This covers the following cases:
    //   - layers that are described w.r.t. a render surface: should have draw transforms described w.r.t. that surface
    //   - A render surface described w.r.t. an ancestor render surface: should have a draw transform described w.r.t. that ancestor surface
    //   - Replicas of a render surface are described w.r.t. the replica's transform around its anchor, along with the surface itself.
    //   - Sanity check on recursion: verify transforms of layers described w.r.t. a render surface that is described w.r.t. an ancestor render surface.
    //   - verifying that each layer has a reference to the correct renderSurface and renderTarget values.

    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<Layer> renderSurface2 = Layer::create();
    scoped_refptr<Layer> childOfRoot = Layer::create();
    scoped_refptr<Layer> childOfRS1 = Layer::create();
    scoped_refptr<Layer> childOfRS2 = Layer::create();
    scoped_refptr<Layer> replicaOfRS1 = Layer::create();
    scoped_refptr<Layer> replicaOfRS2 = Layer::create();
    scoped_refptr<Layer> grandChildOfRoot = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> grandChildOfRS1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> grandChildOfRS2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(parent);
    parent->addChild(renderSurface1);
    parent->addChild(childOfRoot);
    renderSurface1->addChild(childOfRS1);
    renderSurface1->addChild(renderSurface2);
    renderSurface2->addChild(childOfRS2);
    childOfRoot->addChild(grandChildOfRoot);
    childOfRS1->addChild(grandChildOfRS1);
    childOfRS2->addChild(grandChildOfRS2);
    renderSurface1->setReplicaLayer(replicaOfRS1.get());
    renderSurface2->setReplicaLayer(replicaOfRS2.get());

    // In combination with descendantDrawsContent, opacity != 1 forces the layer to have a new renderSurface.
    renderSurface1->setOpacity(0.5);
    renderSurface2->setOpacity(0.33f);

    // One-time setup of root layer
    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(1, 2), false);

    // All layers in the tree are initialized with an anchor at .25 and a size of (10,10).
    // matrix "A" is the composite layer transform used in all layers, centered about the anchor point
    // matrix "B" is the sublayer transform used in all layers, centered about the center position of the layer.
    // matrix "R" is the composite replica transform used in all replica layers.
    //
    // x component tests that layerTransform and sublayerTransform are done in the right order (translation and scale are noncommutative).
    // y component has a translation by 1 for every ancestor, which indicates the "depth" of the layer in the hierarchy.
    WebTransformationMatrix translationToAnchor;
    translationToAnchor.translate(2.5, 0);
    WebTransformationMatrix translationToCenter;
    translationToCenter.translate(5, 5);
    WebTransformationMatrix layerTransform;
    layerTransform.translate(1, 1);
    WebTransformationMatrix sublayerTransform;
    sublayerTransform.scale3d(10, 1, 1);
    WebTransformationMatrix replicaLayerTransform;
    replicaLayerTransform.scale3d(-2, 5, 1);

    WebTransformationMatrix A = translationToAnchor * layerTransform * translationToAnchor.inverse();
    WebTransformationMatrix B = translationToCenter * sublayerTransform * translationToCenter.inverse();
    WebTransformationMatrix R = A * translationToAnchor * replicaLayerTransform * translationToAnchor.inverse();

    gfx::Vector2dF surface1ParentTransformScale = MathUtil::computeTransform2dScaleComponents(A * B);
    WebTransformationMatrix surface1SublayerTransform;
    surface1SublayerTransform.scaleNonUniform(surface1ParentTransformScale.x(), surface1ParentTransformScale.y());

    // SS1 = transform given to the subtree of renderSurface1
    WebTransformationMatrix SS1 = surface1SublayerTransform;
    // S1 = transform to move from renderSurface1 pixels to the layer space of the owning layer
    WebTransformationMatrix S1 = surface1SublayerTransform.inverse();

    gfx::Vector2dF surface2ParentTransformScale = MathUtil::computeTransform2dScaleComponents(SS1 * A * B);
    WebTransformationMatrix surface2SublayerTransform;
    surface2SublayerTransform.scaleNonUniform(surface2ParentTransformScale.x(), surface2ParentTransformScale.y());

    // SS2 = transform given to the subtree of renderSurface2
    WebTransformationMatrix SS2 = surface2SublayerTransform;
    // S2 = transform to move from renderSurface2 pixels to the layer space of the owning layer
    WebTransformationMatrix S2 = surface2SublayerTransform.inverse();

    setLayerPropertiesForTesting(parent.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(renderSurface1.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(renderSurface2.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(childOfRoot.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(childOfRS1.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(childOfRS2.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChildOfRoot.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChildOfRS1.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChildOfRS2.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(replicaOfRS1.get(), replicaLayerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(), false);
    setLayerPropertiesForTesting(replicaOfRS2.get(), replicaLayerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(0, 0), gfx::Size(), false);

    executeCalculateDrawTransformsAndVisibility(root.get());

    // Only layers that are associated with render surfaces should have an actual renderSurface() value.
    //
    ASSERT_TRUE(root->renderSurface());
    ASSERT_FALSE(childOfRoot->renderSurface());
    ASSERT_FALSE(grandChildOfRoot->renderSurface());

    ASSERT_TRUE(renderSurface1->renderSurface());
    ASSERT_FALSE(childOfRS1->renderSurface());
    ASSERT_FALSE(grandChildOfRS1->renderSurface());

    ASSERT_TRUE(renderSurface2->renderSurface());
    ASSERT_FALSE(childOfRS2->renderSurface());
    ASSERT_FALSE(grandChildOfRS2->renderSurface());

    // Verify all renderTarget accessors
    //
    EXPECT_EQ(root, parent->renderTarget());
    EXPECT_EQ(root, childOfRoot->renderTarget());
    EXPECT_EQ(root, grandChildOfRoot->renderTarget());

    EXPECT_EQ(renderSurface1, renderSurface1->renderTarget());
    EXPECT_EQ(renderSurface1, childOfRS1->renderTarget());
    EXPECT_EQ(renderSurface1, grandChildOfRS1->renderTarget());

    EXPECT_EQ(renderSurface2, renderSurface2->renderTarget());
    EXPECT_EQ(renderSurface2, childOfRS2->renderTarget());
    EXPECT_EQ(renderSurface2, grandChildOfRS2->renderTarget());

    // Verify layer draw transforms
    //  note that draw transforms are described with respect to the nearest ancestor render surface
    //  but screen space transforms are described with respect to the root.
    //
    EXPECT_TRANSFORMATION_MATRIX_EQ(A, parent->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A, childOfRoot->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A, grandChildOfRoot->drawTransform());

    EXPECT_TRANSFORMATION_MATRIX_EQ(SS1, renderSurface1->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(SS1 * B * A, childOfRS1->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(SS1 * B * A * B * A, grandChildOfRS1->drawTransform());

    EXPECT_TRANSFORMATION_MATRIX_EQ(SS2, renderSurface2->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(SS2 * B * A, childOfRS2->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(SS2 * B * A * B * A, grandChildOfRS2->drawTransform());

    // Verify layer screen-space transforms
    //
    EXPECT_TRANSFORMATION_MATRIX_EQ(A, parent->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A, childOfRoot->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A, grandChildOfRoot->screenSpaceTransform());

    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A, renderSurface1->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A, childOfRS1->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A * B * A, grandChildOfRS1->screenSpaceTransform());

    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A, renderSurface2->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A * B * A, childOfRS2->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A * B * A * B * A, grandChildOfRS2->screenSpaceTransform());

    // Verify render surface transforms.
    //
    // Draw transform of render surface 1 is described with respect to root.
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * S1, renderSurface1->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * R * S1, renderSurface1->renderSurface()->replicaDrawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * S1, renderSurface1->renderSurface()->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * R * S1, renderSurface1->renderSurface()->replicaScreenSpaceTransform());
    // Draw transform of render surface 2 is described with respect to render surface 1.
    EXPECT_TRANSFORMATION_MATRIX_EQ(SS1 * B * A * S2, renderSurface2->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(SS1 * B * R * S2, renderSurface2->renderSurface()->replicaDrawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * A * S2, renderSurface2->renderSurface()->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(A * B * A * B * R * S2, renderSurface2->renderSurface()->replicaScreenSpaceTransform());

    // Sanity check. If these fail there is probably a bug in the test itself.
    // It is expected that we correctly set up transforms so that the y-component of the screen-space transform
    // encodes the "depth" of the layer in the tree.
    EXPECT_FLOAT_EQ(1, parent->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(2, childOfRoot->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(3, grandChildOfRoot->screenSpaceTransform().m42());

    EXPECT_FLOAT_EQ(2, renderSurface1->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(3, childOfRS1->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(4, grandChildOfRS1->screenSpaceTransform().m42());

    EXPECT_FLOAT_EQ(3, renderSurface2->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(4, childOfRS2->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(5, grandChildOfRS2->screenSpaceTransform().m42());
}

TEST(LayerTreeHostCommonTest, verifyTransformsForFlatteningLayer)
{
    // For layers that flatten their subtree, there should be an orthographic projection
    // (for x and y values) in the middle of the transform sequence. Note that the way the
    // code is currently implemented, it is not expected to use a canonical orthographic
    // projection.

    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> grandChild = make_scoped_refptr(new LayerWithForcedDrawsContent());

    WebTransformationMatrix rotationAboutYAxis;
    rotationAboutYAxis.rotate3d(0, 30, 0);

    const WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child.get(), rotationAboutYAxis, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChild.get(), rotationAboutYAxis, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);

    root->addChild(child);
    child->addChild(grandChild);
    child->setForceRenderSurface(true);

    // No layers in this test should preserve 3d.
    ASSERT_FALSE(root->preserves3D());
    ASSERT_FALSE(child->preserves3D());
    ASSERT_FALSE(grandChild->preserves3D());

    WebTransformationMatrix expectedChildDrawTransform = rotationAboutYAxis;
    WebTransformationMatrix expectedChildScreenSpaceTransform = rotationAboutYAxis;
    WebTransformationMatrix expectedGrandChildDrawTransform = rotationAboutYAxis; // draws onto child's renderSurface
    WebTransformationMatrix expectedGrandChildScreenSpaceTransform = rotationAboutYAxis.to2dTransform() * rotationAboutYAxis;

    executeCalculateDrawTransformsAndVisibility(root.get());

    // The child's drawTransform should have been taken by its surface.
    ASSERT_TRUE(child->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildDrawTransform, child->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildScreenSpaceTransform, child->renderSurface()->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildScreenSpaceTransform, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildDrawTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildScreenSpaceTransform, grandChild->screenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyTransformsForDegenerateIntermediateLayer)
{
    // A layer that is empty in one axis, but not the other, was accidentally skipping a necessary translation.
    // Without that translation, the coordinate space of the layer's drawTransform is incorrect.
    //
    // Normally this isn't a problem, because the layer wouldn't be drawn anyway, but if that layer becomes a renderSurface, then
    // its drawTransform is implicitly inherited by the rest of the subtree, which then is positioned incorrectly as a result.

    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> grandChild = make_scoped_refptr(new LayerWithForcedDrawsContent());

    // The child height is zero, but has non-zero width that should be accounted for while computing drawTransforms.
    const WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 0), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);

    root->addChild(child);
    child->addChild(grandChild);
    child->setForceRenderSurface(true);

    executeCalculateDrawTransformsAndVisibility(root.get());

    ASSERT_TRUE(child->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->renderSurface()->drawTransform()); // This is the real test, the rest are sanity checks.
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, grandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyRenderSurfaceListForRenderSurfaceWithClippedLayer)
{
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child = make_scoped_refptr(new LayerWithForcedDrawsContent());

    const WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(30, 30), gfx::Size(10, 10), false);

    parent->addChild(renderSurface1);
    parent->setMasksToBounds(true);
    renderSurface1->addChild(child);
    renderSurface1->setForceRenderSurface(true);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // The child layer's content is entirely outside the parent's clip rect, so the intermediate
    // render surface should not be listed here, even if it was forced to be created. Render surfaces without children or visible
    // content are unexpected at draw time (e.g. we might try to create a content texture of size 0).
    ASSERT_TRUE(parent->renderSurface());
    ASSERT_FALSE(renderSurface1->renderSurface());
    EXPECT_EQ(1U, renderSurfaceLayerList.size());
}

TEST(LayerTreeHostCommonTest, verifyRenderSurfaceListForTransparentChild)
{
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child = make_scoped_refptr(new LayerWithForcedDrawsContent());

    const WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);

    parent->addChild(renderSurface1);
    renderSurface1->addChild(child);
    renderSurface1->setForceRenderSurface(true);
    renderSurface1->setOpacity(0);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // Since the layer is transparent, renderSurface1->renderSurface() should not have gotten added anywhere.
    // Also, the drawable content rect should not have been extended by the children.
    ASSERT_TRUE(parent->renderSurface());
    EXPECT_EQ(0U, parent->renderSurface()->layerList().size());
    EXPECT_EQ(1U, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());
    EXPECT_EQ(gfx::Rect(), parent->drawableContentRect());
}

TEST(LayerTreeHostCommonTest, verifyForceRenderSurface)
{
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child = make_scoped_refptr(new LayerWithForcedDrawsContent());
    renderSurface1->setForceRenderSurface(true);

    const WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(), gfx::PointF(), gfx::Size(10, 10), false);

    parent->addChild(renderSurface1);
    renderSurface1->addChild(child);

    // Sanity check before the actual test
    EXPECT_FALSE(parent->renderSurface());
    EXPECT_FALSE(renderSurface1->renderSurface());

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // The root layer always creates a renderSurface
    EXPECT_TRUE(parent->renderSurface());
    EXPECT_TRUE(renderSurface1->renderSurface());
    EXPECT_EQ(2U, renderSurfaceLayerList.size());

    renderSurfaceLayerList.clear();
    renderSurface1->setForceRenderSurface(false);
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);
    EXPECT_TRUE(parent->renderSurface());
    EXPECT_FALSE(renderSurface1->renderSurface());
    EXPECT_EQ(1U, renderSurfaceLayerList.size());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithDirectContainer)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // is the direct parent of the fixed-position layer.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];

    child->setIsContainerForFixedPositionLayers(true);
    grandChild->setFixedToContainerLayer(true);

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;
    WebTransformationMatrix expectedGrandChildTransform = expectedChildTransform;

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());

    // Case 2: scrollDelta of 10, 10
    child->setScrollDelta(IntSize(10, 10));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Here the child is affected by scrollDelta, but the fixed position grandChild should not be affected.
    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, -10);

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithTransformedDirectContainer)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // is the direct parent of the fixed-position layer, but that container is transformed.
    // In this case, the fixed position element inherits the container's transform,
    // but the scrollDelta that has to be undone should not be affected by that transform.
    //
    // Transforms are in general non-commutative; using something like a non-uniform scale
    // helps to verify that translations and non-uniform scales are applied in the correct
    // order.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];

    // This scale will cause child and grandChild to be effectively 200 x 800 with respect to the renderTarget.
    WebTransformationMatrix nonUniformScale;
    nonUniformScale.scaleNonUniform(2, 8);
    child->setTransform(nonUniformScale);

    child->setIsContainerForFixedPositionLayers(true);
    grandChild->setFixedToContainerLayer(true);

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;
    expectedChildTransform.multiply(nonUniformScale);

    WebTransformationMatrix expectedGrandChildTransform = expectedChildTransform;

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());

    // Case 2: scrollDelta of 10, 20
    child->setScrollDelta(IntSize(10, 20));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // The child should be affected by scrollDelta, but the fixed position grandChild should not be affected.
    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, -20); // scrollDelta
    expectedChildTransform.multiply(nonUniformScale);

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithDistantContainer)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // is NOT the direct parent of the fixed-position layer.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];
    LayerImpl* greatGrandChild = grandChild->children()[0];

    child->setIsContainerForFixedPositionLayers(true);
    grandChild->setPosition(gfx::PointF(8, 6));
    greatGrandChild->setFixedToContainerLayer(true);

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;
    WebTransformationMatrix expectedGrandChildTransform;
    expectedGrandChildTransform.translate(8, 6);

    WebTransformationMatrix expectedGreatGrandChildTransform = expectedGrandChildTransform;

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());

    // Case 2: scrollDelta of 10, 10
    child->setScrollDelta(IntSize(10, 10));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Here the child and grandChild are affected by scrollDelta, but the fixed position greatGrandChild should not be affected.
    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, -10);
    expectedGrandChildTransform.makeIdentity();
    expectedGrandChildTransform.translate(-2, -4);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithDistantContainerAndTransforms)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // is NOT the direct parent of the fixed-position layer, and the hierarchy has various
    // transforms that have to be processed in the correct order.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];
    LayerImpl* greatGrandChild = grandChild->children()[0];

    WebTransformationMatrix rotationAboutZ;
    rotationAboutZ.rotate3d(0, 0, 90);

    child->setIsContainerForFixedPositionLayers(true);
    child->setTransform(rotationAboutZ);
    grandChild->setPosition(gfx::PointF(8, 6));
    grandChild->setTransform(rotationAboutZ);
    greatGrandChild->setFixedToContainerLayer(true); // greatGrandChild is positioned upside-down with respect to the renderTarget.

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;
    expectedChildTransform.multiply(rotationAboutZ);

    WebTransformationMatrix expectedGrandChildTransform;
    expectedGrandChildTransform.multiply(rotationAboutZ); // child's local transform is inherited
    expectedGrandChildTransform.translate(8, 6); // translation because of position occurs before layer's local transform.
    expectedGrandChildTransform.multiply(rotationAboutZ); // grandChild's local transform

    WebTransformationMatrix expectedGreatGrandChildTransform = expectedGrandChildTransform;

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());

    // Case 2: scrollDelta of 10, 20
    child->setScrollDelta(IntSize(10, 20));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Here the child and grandChild are affected by scrollDelta, but the fixed position greatGrandChild should not be affected.
    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, -20); // scrollDelta
    expectedChildTransform.multiply(rotationAboutZ);

    expectedGrandChildTransform.makeIdentity();
    expectedGrandChildTransform.translate(-10, -20); // child's scrollDelta is inherited
    expectedGrandChildTransform.multiply(rotationAboutZ); // child's local transform is inherited
    expectedGrandChildTransform.translate(8, 6); // translation because of position occurs before layer's local transform.
    expectedGrandChildTransform.multiply(rotationAboutZ); // grandChild's local transform

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithMultipleScrollDeltas)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // has multiple ancestors that have nonzero scrollDelta before reaching the space where the layer is fixed.
    // In this test, each scrollDelta occurs in a different space because of each layer's local transform.
    // This test checks for correct scroll compensation when the fixed-position container
    // is NOT the direct parent of the fixed-position layer, and the hierarchy has various
    // transforms that have to be processed in the correct order.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];
    LayerImpl* greatGrandChild = grandChild->children()[0];

    WebTransformationMatrix rotationAboutZ;
    rotationAboutZ.rotate3d(0, 0, 90);

    child->setIsContainerForFixedPositionLayers(true);
    child->setTransform(rotationAboutZ);
    grandChild->setPosition(gfx::PointF(8, 6));
    grandChild->setTransform(rotationAboutZ);
    greatGrandChild->setFixedToContainerLayer(true); // greatGrandChild is positioned upside-down with respect to the renderTarget.

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;
    expectedChildTransform.multiply(rotationAboutZ);

    WebTransformationMatrix expectedGrandChildTransform;
    expectedGrandChildTransform.multiply(rotationAboutZ); // child's local transform is inherited
    expectedGrandChildTransform.translate(8, 6); // translation because of position occurs before layer's local transform.
    expectedGrandChildTransform.multiply(rotationAboutZ); // grandChild's local transform

    WebTransformationMatrix expectedGreatGrandChildTransform = expectedGrandChildTransform;

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());

    // Case 2: scrollDelta of 10, 20
    child->setScrollDelta(IntSize(10, 0));
    grandChild->setScrollDelta(IntSize(5, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Here the child and grandChild are affected by scrollDelta, but the fixed position greatGrandChild should not be affected.
    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, 0); // scrollDelta
    expectedChildTransform.multiply(rotationAboutZ);

    expectedGrandChildTransform.makeIdentity();
    expectedGrandChildTransform.translate(-10, 0); // child's scrollDelta is inherited
    expectedGrandChildTransform.multiply(rotationAboutZ); // child's local transform is inherited
    expectedGrandChildTransform.translate(-5, 0); // grandChild's scrollDelta
    expectedGrandChildTransform.translate(8, 6); // translation because of position occurs before layer's local transform.
    expectedGrandChildTransform.multiply(rotationAboutZ); // grandChild's local transform

    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithIntermediateSurfaceAndTransforms)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // contributes to a different renderSurface than the fixed-position layer. In this
    // case, the surface drawTransforms also have to be accounted for when checking the
    // scrollDelta.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];
    LayerImpl* greatGrandChild = grandChild->children()[0];

    child->setIsContainerForFixedPositionLayers(true);
    grandChild->setPosition(gfx::PointF(8, 6));
    grandChild->setForceRenderSurface(true);
    greatGrandChild->setFixedToContainerLayer(true);
    greatGrandChild->setDrawsContent(true);

    WebTransformationMatrix rotationAboutZ;
    rotationAboutZ.rotate3d(0, 0, 90);
    grandChild->setTransform(rotationAboutZ);

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;
    WebTransformationMatrix expectedSurfaceDrawTransform;
    expectedSurfaceDrawTransform.translate(8, 6);
    expectedSurfaceDrawTransform.multiply(rotationAboutZ);
    WebTransformationMatrix expectedGrandChildTransform;
    WebTransformationMatrix expectedGreatGrandChildTransform;
    ASSERT_TRUE(grandChild->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedSurfaceDrawTransform, grandChild->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());

    // Case 2: scrollDelta of 10, 30
    child->setScrollDelta(IntSize(10, 30));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Here the grandChild remains unchanged, because it scrolls along with the
    // renderSurface, and the translation is actually in the renderSurface. But, the fixed
    // position greatGrandChild is more awkward: its actually being drawn with respect to
    // the renderSurface, but it needs to remain fixed with resepct to a container beyond
    // that surface. So, the net result is that, unlike previous tests where the fixed
    // position layer's transform remains unchanged, here the fixed position layer's
    // transform explicitly contains the translation that cancels out the scroll.
    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, -30); // scrollDelta

    expectedSurfaceDrawTransform.makeIdentity();
    expectedSurfaceDrawTransform.translate(-10, -30); // scrollDelta
    expectedSurfaceDrawTransform.translate(8, 6);
    expectedSurfaceDrawTransform.multiply(rotationAboutZ);

    // The rotation and its inverse are needed to place the scrollDelta compensation in
    // the correct space. This test will fail if the rotation/inverse are backwards, too,
    // so it requires perfect order of operations.
    expectedGreatGrandChildTransform.makeIdentity();
    expectedGreatGrandChildTransform.multiply(rotationAboutZ.inverse());
    expectedGreatGrandChildTransform.translate(10, 30); // explicit canceling out the scrollDelta that gets embedded in the fixed position layer's surface.
    expectedGreatGrandChildTransform.multiply(rotationAboutZ);

    ASSERT_TRUE(grandChild->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedSurfaceDrawTransform, grandChild->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithMultipleIntermediateSurfaces)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // contributes to a different renderSurface than the fixed-position layer, with
    // additional renderSurfaces in-between. This checks that the conversion to ancestor
    // surfaces is accumulated properly in the final matrix transform.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];
    LayerImpl* greatGrandChild = grandChild->children()[0];

    // Add one more layer to the test tree for this scenario.
    {
        WebTransformationMatrix identity;
        scoped_ptr<LayerImpl> fixedPositionChild = LayerImpl::create(5);
        setLayerPropertiesForTesting(fixedPositionChild.get(), identity, identity, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
        greatGrandChild->addChild(fixedPositionChild.Pass());
    }
    LayerImpl* fixedPositionChild = greatGrandChild->children()[0];

    // Actually set up the scenario here.
    child->setIsContainerForFixedPositionLayers(true);
    grandChild->setPosition(gfx::PointF(8, 6));
    grandChild->setForceRenderSurface(true);
    greatGrandChild->setPosition(gfx::PointF(40, 60));
    greatGrandChild->setForceRenderSurface(true);
    fixedPositionChild->setFixedToContainerLayer(true);
    fixedPositionChild->setDrawsContent(true);

    // The additional rotations, which are non-commutative with translations, help to
    // verify that we have correct order-of-operations in the final scroll compensation.
    // Note that rotating about the center of the layer ensures we do not accidentally
    // clip away layers that we want to test.
    WebTransformationMatrix rotationAboutZ;
    rotationAboutZ.translate(50, 50);
    rotationAboutZ.rotate3d(0, 0, 90);
    rotationAboutZ.translate(-50, -50);
    grandChild->setTransform(rotationAboutZ);
    greatGrandChild->setTransform(rotationAboutZ);

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;

    WebTransformationMatrix expectedGrandChildSurfaceDrawTransform;
    expectedGrandChildSurfaceDrawTransform.translate(8, 6);
    expectedGrandChildSurfaceDrawTransform.multiply(rotationAboutZ);

    WebTransformationMatrix expectedGrandChildTransform;

    WebTransformationMatrix expectedGreatGrandChildSurfaceDrawTransform;
    expectedGreatGrandChildSurfaceDrawTransform.translate(40, 60);
    expectedGreatGrandChildSurfaceDrawTransform.multiply(rotationAboutZ);

    WebTransformationMatrix expectedGreatGrandChildTransform;

    WebTransformationMatrix expectedFixedPositionChildTransform;

    ASSERT_TRUE(grandChild->renderSurface());
    ASSERT_TRUE(greatGrandChild->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildSurfaceDrawTransform, grandChild->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildSurfaceDrawTransform, greatGrandChild->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedFixedPositionChildTransform, fixedPositionChild->drawTransform());

    // Case 2: scrollDelta of 10, 30
    child->setScrollDelta(IntSize(10, 30));
    executeCalculateDrawTransformsAndVisibility(root.get());

    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, -30); // scrollDelta

    expectedGrandChildSurfaceDrawTransform.makeIdentity();
    expectedGrandChildSurfaceDrawTransform.translate(-10, -30); // scrollDelta
    expectedGrandChildSurfaceDrawTransform.translate(8, 6);
    expectedGrandChildSurfaceDrawTransform.multiply(rotationAboutZ);

    // grandChild, greatGrandChild, and greatGrandChild's surface are not expected to
    // change, since they are all not fixed, and they are all drawn with respect to
    // grandChild's surface that already has the scrollDelta accounted for.

    // But the great-great grandchild, "fixedPositionChild", should have a transform that explicitly cancels out the scrollDelta.
    // The expected transform is:
    //   compoundDrawTransform.inverse() * translate(positive scrollDelta) * compoundOriginTransform
    WebTransformationMatrix compoundDrawTransform; // transform from greatGrandChildSurface's origin to the root surface.
    compoundDrawTransform.translate(8, 6); // origin translation of grandChild
    compoundDrawTransform.multiply(rotationAboutZ); // rotation of grandChild
    compoundDrawTransform.translate(40, 60); // origin translation of greatGrandChild
    compoundDrawTransform.multiply(rotationAboutZ); // rotation of greatGrandChild

    expectedFixedPositionChildTransform.makeIdentity();
    expectedFixedPositionChildTransform.multiply(compoundDrawTransform.inverse());
    expectedFixedPositionChildTransform.translate(10, 30); // explicit canceling out the scrollDelta that gets embedded in the fixed position layer's surface.
    expectedFixedPositionChildTransform.multiply(compoundDrawTransform);

    ASSERT_TRUE(grandChild->renderSurface());
    ASSERT_TRUE(greatGrandChild->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildSurfaceDrawTransform, grandChild->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildSurfaceDrawTransform, greatGrandChild->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGreatGrandChildTransform, greatGrandChild->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedFixedPositionChildTransform, fixedPositionChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerWithContainerLayerThatHasSurface)
{
    // This test checks for correct scroll compensation when the fixed-position container
    // itself has a renderSurface. In this case, the container layer should be treated
    // like a layer that contributes to a renderTarget, and that renderTarget
    // is completely irrelevant; it should not affect the scroll compensation.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];

    child->setIsContainerForFixedPositionLayers(true);
    child->setForceRenderSurface(true);
    grandChild->setFixedToContainerLayer(true);
    grandChild->setDrawsContent(true);

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedSurfaceDrawTransform;
    expectedSurfaceDrawTransform.translate(0, 0);
    WebTransformationMatrix expectedChildTransform;
    WebTransformationMatrix expectedGrandChildTransform;
    ASSERT_TRUE(child->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedSurfaceDrawTransform, child->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());

    // Case 2: scrollDelta of 10, 10
    child->setScrollDelta(IntSize(10, 10));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // The surface is translated by scrollDelta, the child transform doesn't change
    // because it scrolls along with the surface, but the fixed position grandChild
    // needs to compensate for the scroll translation.
    expectedSurfaceDrawTransform.makeIdentity();
    expectedSurfaceDrawTransform.translate(-10, -10);
    expectedGrandChildTransform.makeIdentity();
    expectedGrandChildTransform.translate(10, 10);

    ASSERT_TRUE(child->renderSurface());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedSurfaceDrawTransform, child->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerThatIsAlsoFixedPositionContainer)
{
    // This test checks the scenario where a fixed-position layer also happens to be a
    // container itself for a descendant fixed position layer. In particular, the layer
    // should not accidentally be fixed to itself.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];

    child->setIsContainerForFixedPositionLayers(true);
    grandChild->setFixedToContainerLayer(true);

    // This should not confuse the grandChild. If correct, the grandChild would still be considered fixed to its container (i.e. "child").
    grandChild->setIsContainerForFixedPositionLayers(true);

    // Case 1: scrollDelta of 0, 0
    child->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix expectedChildTransform;
    WebTransformationMatrix expectedGrandChildTransform;
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());

    // Case 2: scrollDelta of 10, 10
    child->setScrollDelta(IntSize(10, 10));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // Here the child is affected by scrollDelta, but the fixed position grandChild should not be affected.
    expectedChildTransform.makeIdentity();
    expectedChildTransform.translate(-10, -10);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyScrollCompensationForFixedPositionLayerThatHasNoContainer)
{
    // This test checks scroll compensation when a fixed-position layer does not find any
    // ancestor that is a "containerForFixedPositionLayers". In this situation, the layer should
    // be fixed to the viewport -- not the rootLayer, which may have transforms of its own.
    scoped_ptr<LayerImpl> root = createTreeForFixedPositionTests();
    LayerImpl* child = root->children()[0];
    LayerImpl* grandChild = child->children()[0];

    WebTransformationMatrix rotationByZ;
    rotationByZ.rotate3d(0, 0, 90);

    root->setTransform(rotationByZ);
    grandChild->setFixedToContainerLayer(true);

    // Case 1: root scrollDelta of 0, 0
    root->setScrollDelta(IntSize(0, 0));
    executeCalculateDrawTransformsAndVisibility(root.get());

    WebTransformationMatrix identityMatrix;

    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, grandChild->drawTransform());

    // Case 2: root scrollDelta of 10, 10
    root->setScrollDelta(IntSize(10, 20));
    executeCalculateDrawTransformsAndVisibility(root.get());

    // The child is affected by scrollDelta, but it is already implcitly accounted for by
    // the child's target surface (i.e. the root renderSurface). The grandChild is not
    // affected by the scrollDelta, so its drawTransform needs to explicitly
    // inverse-compensate for the scroll that's embedded in the target surface.
    WebTransformationMatrix expectedGrandChildTransform;
    expectedGrandChildTransform.multiply(rotationByZ.inverse());
    expectedGrandChildTransform.translate(10, 20); // explicit canceling out the scrollDelta that gets embedded in the fixed position layer's surface.
    expectedGrandChildTransform.multiply(rotationByZ);

    EXPECT_TRANSFORMATION_MATRIX_EQ(identityMatrix, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedGrandChildTransform, grandChild->drawTransform());
}

TEST(LayerTreeHostCommonTest, verifyClipRectCullsRenderSurfaces)
{
    // The entire subtree of layers that are outside the clipRect should be culled away,
    // and should not affect the renderSurfaceLayerList.
    //
    // The test tree is set up as follows:
    //  - all layers except the leafNodes are forced to be a new renderSurface that have something to draw.
    //  - parent is a large container layer.
    //  - child has masksToBounds=true to cause clipping.
    //  - grandChild is positioned outside of the child's bounds
    //  - greatGrandChild is also kept outside child's bounds.
    //
    // In this configuration, grandChild and greatGrandChild are completely outside the
    // clipRect, and they should never get scheduled on the list of renderSurfaces.
    //

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> grandChild = Layer::create();
    scoped_refptr<Layer> greatGrandChild = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> leafNode1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> leafNode2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    parent->addChild(child);
    child->addChild(grandChild);
    grandChild->addChild(greatGrandChild);

    // leafNode1 ensures that parent and child are kept on the renderSurfaceLayerList,
    // even though grandChild and greatGrandChild should be clipped.
    child->addChild(leafNode1);
    greatGrandChild->addChild(leafNode2);

    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(500, 500), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(20, 20), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(45, 45), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(greatGrandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(leafNode1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(500, 500), false);
    setLayerPropertiesForTesting(leafNode2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(20, 20), false);

    child->setMasksToBounds(true);
    child->setOpacity(0.4f);
    grandChild->setOpacity(0.5);
    greatGrandChild->setOpacity(0.4f);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    ASSERT_EQ(2U, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());
    EXPECT_EQ(child->id(), renderSurfaceLayerList[1]->id());
}

TEST(LayerTreeHostCommonTest, verifyClipRectCullsSurfaceWithoutVisibleContent)
{
    // When a renderSurface has a clipRect, it is used to clip the contentRect
    // of the surface. When the renderSurface is animating its transforms, then
    // the contentRect's position in the clipRect is not defined on the main
    // thread, and its contentRect should not be clipped.

    // The test tree is set up as follows:
    //  - parent is a container layer that masksToBounds=true to cause clipping.
    //  - child is a renderSurface, which has a clipRect set to the bounds of the parent.
    //  - grandChild is a renderSurface, and the only visible content in child. It is positioned outside of the clipRect from parent.

    // In this configuration, grandChild should be outside the clipped
    // contentRect of the child, making grandChild not appear in the
    // renderSurfaceLayerList. However, when we place an animation on the child,
    // this clipping should be avoided and we should keep the grandChild
    // in the renderSurfaceLayerList.

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> grandChild = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> leafNode = make_scoped_refptr(new LayerWithForcedDrawsContent());
    parent->addChild(child);
    child->addChild(grandChild);
    grandChild->addChild(leafNode);

    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(20, 20), false);
    setLayerPropertiesForTesting(grandChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(200, 200), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(leafNode.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);

    parent->setMasksToBounds(true);
    child->setOpacity(0.4f);
    grandChild->setOpacity(0.4f);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // Without an animation, we should cull child and grandChild from the renderSurfaceLayerList.
    ASSERT_EQ(1U, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());

    // Now put an animating transform on child.
    addAnimatedTransformToController(*child->layerAnimationController(), 10, 30, 0);

    parent->clearRenderSurface();
    child->clearRenderSurface();
    grandChild->clearRenderSurface();
    renderSurfaceLayerList.clear();

    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // With an animating transform, we should keep child and grandChild in the renderSurfaceLayerList.
    ASSERT_EQ(3U, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());
    EXPECT_EQ(child->id(), renderSurfaceLayerList[1]->id());
    EXPECT_EQ(grandChild->id(), renderSurfaceLayerList[2]->id());
}

TEST(LayerTreeHostCommonTest, verifyDrawableContentRectForLayers)
{
    // Verify that layers get the appropriate drawableContentRect when their parent masksToBounds is true.
    //
    //   grandChild1 - completely inside the region; drawableContentRect should be the layer rect expressed in target space.
    //   grandChild2 - partially clipped but NOT masksToBounds; the clipRect will be the intersection of layerBounds and the mask region.
    //   grandChild3 - partially clipped and masksToBounds; the drawableContentRect will still be the intersection of layerBounds and the mask region.
    //   grandChild4 - outside parent's clipRect; the drawableContentRect should be empty.
    //

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> grandChild1 = Layer::create();
    scoped_refptr<Layer> grandChild2 = Layer::create();
    scoped_refptr<Layer> grandChild3 = Layer::create();
    scoped_refptr<Layer> grandChild4 = Layer::create();

    parent->addChild(child);
    child->addChild(grandChild1);
    child->addChild(grandChild2);
    child->addChild(grandChild3);
    child->addChild(grandChild4);

    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(500, 500), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(20, 20), false);
    setLayerPropertiesForTesting(grandChild1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChild2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(15, 15), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChild3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(15, 15), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChild4.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(45, 45), gfx::Size(10, 10), false);

    child->setMasksToBounds(true);
    grandChild3->setMasksToBounds(true);

    // Force everyone to be a render surface.
    child->setOpacity(0.4f);
    grandChild1->setOpacity(0.5);
    grandChild2->setOpacity(0.5);
    grandChild3->setOpacity(0.5);
    grandChild4->setOpacity(0.5);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_RECT_EQ(gfx::Rect(gfx::Point(5, 5), gfx::Size(10, 10)), grandChild1->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(gfx::Point(15, 15), gfx::Size(5, 5)), grandChild3->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(gfx::Point(15, 15), gfx::Size(5, 5)), grandChild3->drawableContentRect());
    EXPECT_TRUE(grandChild4->drawableContentRect().IsEmpty());
}

TEST(LayerTreeHostCommonTest, verifyClipRectIsPropagatedCorrectlyToSurfaces)
{
    // Verify that renderSurfaces (and their layers) get the appropriate clipRects when their parent masksToBounds is true.
    //
    // Layers that own renderSurfaces (at least for now) do not inherit any clipping;
    // instead the surface will enforce the clip for the entire subtree. They may still
    // have a clipRect of their own layer bounds, however, if masksToBounds was true.
    //

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> grandChild1 = Layer::create();
    scoped_refptr<Layer> grandChild2 = Layer::create();
    scoped_refptr<Layer> grandChild3 = Layer::create();
    scoped_refptr<Layer> grandChild4 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> leafNode1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> leafNode2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> leafNode3 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> leafNode4 = make_scoped_refptr(new LayerWithForcedDrawsContent());

    parent->addChild(child);
    child->addChild(grandChild1);
    child->addChild(grandChild2);
    child->addChild(grandChild3);
    child->addChild(grandChild4);

    // the leaf nodes ensure that these grandChildren become renderSurfaces for this test.
    grandChild1->addChild(leafNode1);
    grandChild2->addChild(leafNode2);
    grandChild3->addChild(leafNode3);
    grandChild4->addChild(leafNode4);

    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(500, 500), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(20, 20), false);
    setLayerPropertiesForTesting(grandChild1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChild2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(15, 15), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChild3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(15, 15), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChild4.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(45, 45), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(leafNode1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(leafNode2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(leafNode3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(leafNode4.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), false);

    child->setMasksToBounds(true);
    grandChild3->setMasksToBounds(true);
    grandChild4->setMasksToBounds(true);

    // Force everyone to be a render surface.
    child->setOpacity(0.4f);
    grandChild1->setOpacity(0.5);
    grandChild2->setOpacity(0.5);
    grandChild3->setOpacity(0.5);
    grandChild4->setOpacity(0.5);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    ASSERT_TRUE(grandChild1->renderSurface());
    ASSERT_TRUE(grandChild2->renderSurface());
    ASSERT_TRUE(grandChild3->renderSurface());
    EXPECT_FALSE(grandChild4->renderSurface()); // Because grandChild4 is entirely clipped, it is expected to not have a renderSurface.

    // Surfaces are clipped by their parent, but un-affected by the owning layer's masksToBounds.
    EXPECT_RECT_EQ(gfx::Rect(gfx::Point(0, 0), gfx::Size(20, 20)), grandChild1->renderSurface()->clipRect());
    EXPECT_RECT_EQ(gfx::Rect(gfx::Point(0, 0), gfx::Size(20, 20)), grandChild2->renderSurface()->clipRect());
    EXPECT_RECT_EQ(gfx::Rect(gfx::Point(0, 0), gfx::Size(20, 20)), grandChild3->renderSurface()->clipRect());
}

TEST(LayerTreeHostCommonTest, verifyAnimationsForRenderSurfaceHierarchy)
{
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<Layer> renderSurface2 = Layer::create();
    scoped_refptr<Layer> childOfRoot = Layer::create();
    scoped_refptr<Layer> childOfRS1 = Layer::create();
    scoped_refptr<Layer> childOfRS2 = Layer::create();
    scoped_refptr<Layer> grandChildOfRoot = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> grandChildOfRS1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> grandChildOfRS2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    parent->addChild(renderSurface1);
    parent->addChild(childOfRoot);
    renderSurface1->addChild(childOfRS1);
    renderSurface1->addChild(renderSurface2);
    renderSurface2->addChild(childOfRS2);
    childOfRoot->addChild(grandChildOfRoot);
    childOfRS1->addChild(grandChildOfRS1);
    childOfRS2->addChild(grandChildOfRS2);

    // Make our render surfaces.
    renderSurface1->setForceRenderSurface(true);
    renderSurface2->setForceRenderSurface(true);

    // Put an animated opacity on the render surface.
    addOpacityTransitionToController(*renderSurface1->layerAnimationController(), 10, 1, 0, false);

    // Also put an animated opacity on a layer without descendants.
    addOpacityTransitionToController(*grandChildOfRoot->layerAnimationController(), 10, 1, 0, false);

    WebTransformationMatrix layerTransform;
    layerTransform.translate(1, 1);
    WebTransformationMatrix sublayerTransform;
    sublayerTransform.scale3d(10, 1, 1);

    // Put a transform animation on the render surface.
    addAnimatedTransformToController(*renderSurface2->layerAnimationController(), 10, 30, 0);

    // Also put transform animations on grandChildOfRoot, and grandChildOfRS2
    addAnimatedTransformToController(*grandChildOfRoot->layerAnimationController(), 10, 30, 0);
    addAnimatedTransformToController(*grandChildOfRS2->layerAnimationController(), 10, 30, 0);

    setLayerPropertiesForTesting(parent.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(renderSurface1.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(renderSurface2.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(childOfRoot.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(childOfRS1.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(childOfRS2.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChildOfRoot.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChildOfRS1.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);
    setLayerPropertiesForTesting(grandChildOfRS2.get(), layerTransform, sublayerTransform, gfx::PointF(0.25, 0), gfx::PointF(2.5, 0), gfx::Size(10, 10), false);

    executeCalculateDrawTransformsAndVisibility(parent.get());

    // Only layers that are associated with render surfaces should have an actual renderSurface() value.
    //
    ASSERT_TRUE(parent->renderSurface());
    ASSERT_FALSE(childOfRoot->renderSurface());
    ASSERT_FALSE(grandChildOfRoot->renderSurface());

    ASSERT_TRUE(renderSurface1->renderSurface());
    ASSERT_FALSE(childOfRS1->renderSurface());
    ASSERT_FALSE(grandChildOfRS1->renderSurface());

    ASSERT_TRUE(renderSurface2->renderSurface());
    ASSERT_FALSE(childOfRS2->renderSurface());
    ASSERT_FALSE(grandChildOfRS2->renderSurface());

    // Verify all renderTarget accessors
    //
    EXPECT_EQ(parent, parent->renderTarget());
    EXPECT_EQ(parent, childOfRoot->renderTarget());
    EXPECT_EQ(parent, grandChildOfRoot->renderTarget());

    EXPECT_EQ(renderSurface1, renderSurface1->renderTarget());
    EXPECT_EQ(renderSurface1, childOfRS1->renderTarget());
    EXPECT_EQ(renderSurface1, grandChildOfRS1->renderTarget());

    EXPECT_EQ(renderSurface2, renderSurface2->renderTarget());
    EXPECT_EQ(renderSurface2, childOfRS2->renderTarget());
    EXPECT_EQ(renderSurface2, grandChildOfRS2->renderTarget());

    // Verify drawOpacityIsAnimating values
    //
    EXPECT_FALSE(parent->drawOpacityIsAnimating());
    EXPECT_FALSE(childOfRoot->drawOpacityIsAnimating());
    EXPECT_TRUE(grandChildOfRoot->drawOpacityIsAnimating());
    EXPECT_FALSE(renderSurface1->drawOpacityIsAnimating());
    EXPECT_TRUE(renderSurface1->renderSurface()->drawOpacityIsAnimating());
    EXPECT_FALSE(childOfRS1->drawOpacityIsAnimating());
    EXPECT_FALSE(grandChildOfRS1->drawOpacityIsAnimating());
    EXPECT_FALSE(renderSurface2->drawOpacityIsAnimating());
    EXPECT_FALSE(renderSurface2->renderSurface()->drawOpacityIsAnimating());
    EXPECT_FALSE(childOfRS2->drawOpacityIsAnimating());
    EXPECT_FALSE(grandChildOfRS2->drawOpacityIsAnimating());

    // Verify drawTransformsAnimatingInTarget values
    //
    EXPECT_FALSE(parent->drawTransformIsAnimating());
    EXPECT_FALSE(childOfRoot->drawTransformIsAnimating());
    EXPECT_TRUE(grandChildOfRoot->drawTransformIsAnimating());
    EXPECT_FALSE(renderSurface1->drawTransformIsAnimating());
    EXPECT_FALSE(renderSurface1->renderSurface()->targetSurfaceTransformsAreAnimating());
    EXPECT_FALSE(childOfRS1->drawTransformIsAnimating());
    EXPECT_FALSE(grandChildOfRS1->drawTransformIsAnimating());
    EXPECT_FALSE(renderSurface2->drawTransformIsAnimating());
    EXPECT_TRUE(renderSurface2->renderSurface()->targetSurfaceTransformsAreAnimating());
    EXPECT_FALSE(childOfRS2->drawTransformIsAnimating());
    EXPECT_TRUE(grandChildOfRS2->drawTransformIsAnimating());

    // Verify drawTransformsAnimatingInScreen values
    //
    EXPECT_FALSE(parent->screenSpaceTransformIsAnimating());
    EXPECT_FALSE(childOfRoot->screenSpaceTransformIsAnimating());
    EXPECT_TRUE(grandChildOfRoot->screenSpaceTransformIsAnimating());
    EXPECT_FALSE(renderSurface1->screenSpaceTransformIsAnimating());
    EXPECT_FALSE(renderSurface1->renderSurface()->screenSpaceTransformsAreAnimating());
    EXPECT_FALSE(childOfRS1->screenSpaceTransformIsAnimating());
    EXPECT_FALSE(grandChildOfRS1->screenSpaceTransformIsAnimating());
    EXPECT_TRUE(renderSurface2->screenSpaceTransformIsAnimating());
    EXPECT_TRUE(renderSurface2->renderSurface()->screenSpaceTransformsAreAnimating());
    EXPECT_TRUE(childOfRS2->screenSpaceTransformIsAnimating());
    EXPECT_TRUE(grandChildOfRS2->screenSpaceTransformIsAnimating());


    // Sanity check. If these fail there is probably a bug in the test itself.
    // It is expected that we correctly set up transforms so that the y-component of the screen-space transform
    // encodes the "depth" of the layer in the tree.
    EXPECT_FLOAT_EQ(1, parent->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(2, childOfRoot->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(3, grandChildOfRoot->screenSpaceTransform().m42());

    EXPECT_FLOAT_EQ(2, renderSurface1->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(3, childOfRS1->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(4, grandChildOfRS1->screenSpaceTransform().m42());

    EXPECT_FLOAT_EQ(3, renderSurface2->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(4, childOfRS2->screenSpaceTransform().m42());
    EXPECT_FLOAT_EQ(5, grandChildOfRS2->screenSpaceTransform().m42());
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectForIdentityTransform)
{
    // Test the calculateVisibleRect() function works correctly for identity transforms.

    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    WebTransformationMatrix layerToSurfaceTransform;

    // Case 1: Layer is contained within the surface.
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(10, 10), gfx::Size(30, 30));
    gfx::Rect expected = gfx::Rect(gfx::Point(10, 10), gfx::Size(30, 30));
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);

    // Case 2: Layer is outside the surface rect.
    layerContentRect = gfx::Rect(gfx::Point(120, 120), gfx::Size(30, 30));
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_TRUE(actual.IsEmpty());

    // Case 3: Layer is partially overlapping the surface rect.
    layerContentRect = gfx::Rect(gfx::Point(80, 80), gfx::Size(30, 30));
    expected = gfx::Rect(gfx::Point(80, 80), gfx::Size(20, 20));
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectForTranslations)
{
    // Test the calculateVisibleRect() function works correctly for scaling transforms.

    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(30, 30));
    WebTransformationMatrix layerToSurfaceTransform;

    // Case 1: Layer is contained within the surface.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(10, 10);
    gfx::Rect expected = gfx::Rect(gfx::Point(0, 0), gfx::Size(30, 30));
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);

    // Case 2: Layer is outside the surface rect.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(120, 120);
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_TRUE(actual.IsEmpty());

    // Case 3: Layer is partially overlapping the surface rect.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(80, 80);
    expected = gfx::Rect(gfx::Point(0, 0), gfx::Size(20, 20));
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectFor2DRotations)
{
    // Test the calculateVisibleRect() function works correctly for rotations about z-axis (i.e. 2D rotations).
    // Remember that calculateVisibleRect() should return the visible rect in the layer's space.

    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(30, 30));
    WebTransformationMatrix layerToSurfaceTransform;

    // Case 1: Layer is contained within the surface.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(50, 50);
    layerToSurfaceTransform.rotate(45);
    gfx::Rect expected = gfx::Rect(gfx::Point(0, 0), gfx::Size(30, 30));
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);

    // Case 2: Layer is outside the surface rect.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(-50, 0);
    layerToSurfaceTransform.rotate(45);
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_TRUE(actual.IsEmpty());

    // Case 3: The layer is rotated about its top-left corner. In surface space, the layer
    //         is oriented diagonally, with the left half outside of the renderSurface. In
    //         this case, the visible rect should still be the entire layer (remember the
    //         visible rect is computed in layer space); both the top-left and
    //         bottom-right corners of the layer are still visible.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.rotate(45);
    expected = gfx::Rect(gfx::Point(0, 0), gfx::Size(30, 30));
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);

    // Case 4: The layer is rotated about its top-left corner, and translated upwards. In
    //         surface space, the layer is oriented diagonally, with only the top corner
    //         of the surface overlapping the layer. In layer space, the render surface
    //         overlaps the right side of the layer. The visible rect should be the
    //         layer's right half.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(0, -sqrt(2.0) * 15);
    layerToSurfaceTransform.rotate(45);
    expected = gfx::Rect(gfx::Point(15, 0), gfx::Size(15, 30)); // right half of layer bounds.
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectFor3dOrthographicTransform)
{
    // Test that the calculateVisibleRect() function works correctly for 3d transforms.

    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    WebTransformationMatrix layerToSurfaceTransform;

    // Case 1: Orthographic projection of a layer rotated about y-axis by 45 degrees, should be fully contained in the renderSurface.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.rotate3d(0, 45, 0);
    gfx::Rect expected = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);

    // Case 2: Orthographic projection of a layer rotated about y-axis by 45 degrees, but
    //         shifted to the side so only the right-half the layer would be visible on
    //         the surface.
    double halfWidthOfRotatedLayer = (100 / sqrt(2.0)) * 0.5; // 100 is the un-rotated layer width; divided by sqrt(2) is the rotated width.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(-halfWidthOfRotatedLayer, 0);
    layerToSurfaceTransform.rotate3d(0, 45, 0); // rotates about the left edge of the layer
    expected = gfx::Rect(gfx::Point(50, 0), gfx::Size(50, 100)); // right half of the layer.
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectFor3dPerspectiveTransform)
{
    // Test the calculateVisibleRect() function works correctly when the layer has a
    // perspective projection onto the target surface.

    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(-50, -50), gfx::Size(200, 200));
    WebTransformationMatrix layerToSurfaceTransform;

    // Case 1: Even though the layer is twice as large as the surface, due to perspective
    //         foreshortening, the layer will fit fully in the surface when its translated
    //         more than the perspective amount.
    layerToSurfaceTransform.makeIdentity();

    // The following sequence of transforms applies the perspective about the center of the surface.
    layerToSurfaceTransform.translate(50, 50);
    layerToSurfaceTransform.applyPerspective(9);
    layerToSurfaceTransform.translate(-50, -50);

    // This translate places the layer in front of the surface's projection plane.
    layerToSurfaceTransform.translate3d(0, 0, -27);

    gfx::Rect expected = gfx::Rect(gfx::Point(-50, -50), gfx::Size(200, 200));
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);

    // Case 2: same projection as before, except that the layer is also translated to the
    //         side, so that only the right half of the layer should be visible.
    //
    // Explanation of expected result:
    // The perspective ratio is (z distance between layer and camera origin) / (z distance between projection plane and camera origin) == ((-27 - 9) / 9)
    // Then, by similar triangles, if we want to move a layer by translating -50 units in projected surface units (so that only half of it is
    // visible), then we would need to translate by (-36 / 9) * -50 == -200 in the layer's units.
    //
    layerToSurfaceTransform.translate3d(-200, 0, 0);
    expected = gfx::Rect(gfx::Point(50, -50), gfx::Size(100, 200)); // The right half of the layer's bounding rect.
    actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectFor3dOrthographicIsNotClippedBehindSurface)
{
    // There is currently no explicit concept of an orthographic projection plane in our
    // code (nor in the CSS spec to my knowledge). Therefore, layers that are technically
    // behind the surface in an orthographic world should not be clipped when they are
    // flattened to the surface.

    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    WebTransformationMatrix layerToSurfaceTransform;

    // This sequence of transforms effectively rotates the layer about the y-axis at the
    // center of the layer.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.translate(50, 0);
    layerToSurfaceTransform.rotate3d(0, 45, 0);
    layerToSurfaceTransform.translate(-50, 0);

    gfx::Rect expected = gfx::Rect(gfx::Point(0, 0), gfx::Size(100, 100));
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectFor3dPerspectiveWhenClippedByW)
{
    // Test the calculateVisibleRect() function works correctly when projecting a surface
    // onto a layer, but the layer is partially behind the camera (not just behind the
    // projection plane). In this case, the cartesian coordinates may seem to be valid,
    // but actually they are not. The visibleRect needs to be properly clipped by the
    // w = 0 plane in homogeneous coordinates before converting to cartesian coordinates.

    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(-50, -50), gfx::Size(100, 100));
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(-10, -1), gfx::Size(20, 2));
    WebTransformationMatrix layerToSurfaceTransform;

    // The layer is positioned so that the right half of the layer should be in front of
    // the camera, while the other half is behind the surface's projection plane. The
    // following sequence of transforms applies the perspective and rotation about the
    // center of the layer.
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.applyPerspective(1);
    layerToSurfaceTransform.translate3d(-2, 0, 1);
    layerToSurfaceTransform.rotate3d(0, 45, 0);

    // Sanity check that this transform does indeed cause w < 0 when applying the
    // transform, otherwise this code is not testing the intended scenario.
    bool clipped = false;
    MathUtil::mapQuad(layerToSurfaceTransform, gfx::QuadF(gfx::RectF(layerContentRect)), clipped);
    ASSERT_TRUE(clipped);

    int expectedXPosition = 0;
    int expectedWidth = 10;
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_EQ(expectedXPosition, actual.x());
    EXPECT_EQ(expectedWidth, actual.width());
}

TEST(LayerTreeHostCommonTest, verifyVisibleRectForPerspectiveUnprojection)
{
    // To determine visibleRect in layer space, there needs to be an un-projection from
    // surface space to layer space. When the original transform was a perspective
    // projection that was clipped, it returns a rect that encloses the clipped bounds.
    // Un-projecting this new rect may require clipping again.

    // This sequence of transforms causes one corner of the layer to protrude across the w = 0 plane, and should be clipped.
    gfx::Rect targetSurfaceRect = gfx::Rect(gfx::Point(-50, -50), gfx::Size(100, 100));
    gfx::Rect layerContentRect = gfx::Rect(gfx::Point(-10, -10), gfx::Size(20, 20));
    WebTransformationMatrix layerToSurfaceTransform;
    layerToSurfaceTransform.makeIdentity();
    layerToSurfaceTransform.applyPerspective(1);
    layerToSurfaceTransform.translate3d(0, 0, -5);
    layerToSurfaceTransform.rotate3d(0, 45, 0);
    layerToSurfaceTransform.rotate3d(80, 0, 0);

    // Sanity check that un-projection does indeed cause w < 0, otherwise this code is not
    // testing the intended scenario.
    bool clipped = false;
    gfx::RectF clippedRect = MathUtil::mapClippedRect(layerToSurfaceTransform, layerContentRect);
    MathUtil::projectQuad(layerToSurfaceTransform.inverse(), gfx::QuadF(clippedRect), clipped);
    ASSERT_TRUE(clipped);

    // Only the corner of the layer is not visible on the surface because of being
    // clipped. But, the net result of rounding visible region to an axis-aligned rect is
    // that the entire layer should still be considered visible.
    gfx::Rect expected = gfx::Rect(gfx::Point(-10, -10), gfx::Size(20, 20));
    gfx::Rect actual = LayerTreeHostCommon::calculateVisibleRect(targetSurfaceRect, layerContentRect, layerToSurfaceTransform);
    EXPECT_RECT_EQ(expected, actual);
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsForSimpleLayers)
{
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child3 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(child1);
    root->addChild(child2);
    root->addChild(child3);

    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(75, 75), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(125, 125), gfx::Size(50, 50), false);

    executeCalculateDrawTransformsAndVisibility(root.get());

    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->renderSurface()->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->drawableContentRect());

    // Layers that do not draw content should have empty visibleContentRects.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), root->visibleContentRect());

    // layer visibleContentRects are clipped by their targetSurface
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 25, 25), child2->visibleContentRect());
    EXPECT_TRUE(child3->visibleContentRect().IsEmpty());

    // layer drawableContentRects are not clipped.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child1->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(75, 75, 50, 50), child2->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(125, 125, 50, 50), child3->drawableContentRect());
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsForLayersClippedByLayer)
{
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> grandChild1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> grandChild2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> grandChild3 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(child);
    child->addChild(grandChild1);
    child->addChild(grandChild2);
    child->addChild(grandChild3);

    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(grandChild1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(grandChild2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(75, 75), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(grandChild3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(125, 125), gfx::Size(50, 50), false);

    child->setMasksToBounds(true);
    executeCalculateDrawTransformsAndVisibility(root.get());

    ASSERT_FALSE(child->renderSurface());

    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->renderSurface()->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->drawableContentRect());

    // Layers that do not draw content should have empty visibleContentRects.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), root->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), child->visibleContentRect());

    // All grandchild visibleContentRects should be clipped by child.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), grandChild1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 25, 25), grandChild2->visibleContentRect());
    EXPECT_TRUE(grandChild3->visibleContentRect().IsEmpty());

    // All grandchild drawableContentRects should also be clipped by child.
    EXPECT_RECT_EQ(gfx::Rect(5, 5, 50, 50), grandChild1->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(75, 75, 25, 25), grandChild2->drawableContentRect());
    EXPECT_TRUE(grandChild3->drawableContentRect().IsEmpty());
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsForLayersInUnclippedRenderSurface)
{
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child3 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(renderSurface1);
    renderSurface1->addChild(child1);
    renderSurface1->addChild(child2);
    renderSurface1->addChild(child3);

    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(3, 4), false);
    setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(75, 75), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(125, 125), gfx::Size(50, 50), false);

    renderSurface1->setForceRenderSurface(true);
    executeCalculateDrawTransformsAndVisibility(root.get());

    ASSERT_TRUE(renderSurface1->renderSurface());

    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->renderSurface()->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->drawableContentRect());

    // Layers that do not draw content should have empty visibleContentRects.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), root->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), renderSurface1->visibleContentRect());

    // An unclipped surface grows its drawableContentRect to include all drawable regions of the subtree.
    EXPECT_RECT_EQ(gfx::Rect(5, 5, 170, 170), renderSurface1->renderSurface()->drawableContentRect());

    // All layers that draw content into the unclipped surface are also unclipped.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child2->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child3->visibleContentRect());

    EXPECT_RECT_EQ(gfx::Rect(5, 5, 50, 50), child1->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(75, 75, 50, 50), child2->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(125, 125, 50, 50), child3->drawableContentRect());
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsForLayersInClippedRenderSurface)
{
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child3 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(renderSurface1);
    renderSurface1->addChild(child1);
    renderSurface1->addChild(child2);
    renderSurface1->addChild(child3);

    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(3, 4), false);
    setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(75, 75), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(125, 125), gfx::Size(50, 50), false);

    root->setMasksToBounds(true);
    renderSurface1->setForceRenderSurface(true);
    executeCalculateDrawTransformsAndVisibility(root.get());

    ASSERT_TRUE(renderSurface1->renderSurface());

    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->renderSurface()->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->drawableContentRect());

    // Layers that do not draw content should have empty visibleContentRects.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), root->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), renderSurface1->visibleContentRect());

    // A clipped surface grows its drawableContentRect to include all drawable regions of the subtree,
    // but also gets clamped by the ancestor's clip.
    EXPECT_RECT_EQ(gfx::Rect(5, 5, 95, 95), renderSurface1->renderSurface()->drawableContentRect());

    // All layers that draw content into the surface have their visibleContentRect clipped by the surface clipRect.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 25, 25), child2->visibleContentRect());
    EXPECT_TRUE(child3->visibleContentRect().IsEmpty());

    // But the drawableContentRects are unclipped.
    EXPECT_RECT_EQ(gfx::Rect(5, 5, 50, 50), child1->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(75, 75, 50, 50), child2->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(125, 125, 50, 50), child3->drawableContentRect());
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsForSurfaceHierarchy)
{
    // Check that clipping does not propagate down surfaces.
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<Layer> renderSurface2 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child2 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child3 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(renderSurface1);
    renderSurface1->addChild(renderSurface2);
    renderSurface2->addChild(child1);
    renderSurface2->addChild(child2);
    renderSurface2->addChild(child3);

    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(3, 4), false);
    setLayerPropertiesForTesting(renderSurface2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(7, 13), false);
    setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(75, 75), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(125, 125), gfx::Size(50, 50), false);

    root->setMasksToBounds(true);
    renderSurface1->setForceRenderSurface(true);
    renderSurface2->setForceRenderSurface(true);
    executeCalculateDrawTransformsAndVisibility(root.get());

    ASSERT_TRUE(renderSurface1->renderSurface());
    ASSERT_TRUE(renderSurface2->renderSurface());

    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->renderSurface()->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->drawableContentRect());

    // Layers that do not draw content should have empty visibleContentRects.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), root->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), renderSurface1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), renderSurface2->visibleContentRect());

    // A clipped surface grows its drawableContentRect to include all drawable regions of the subtree,
    // but also gets clamped by the ancestor's clip.
    EXPECT_RECT_EQ(gfx::Rect(5, 5, 95, 95), renderSurface1->renderSurface()->drawableContentRect());

    // renderSurface1 lives in the "unclipped universe" of renderSurface1, and is only
    // implicitly clipped by renderSurface1's contentRect. So, renderSurface2 grows to
    // enclose all drawable content of its subtree.
    EXPECT_RECT_EQ(gfx::Rect(5, 5, 170, 170), renderSurface2->renderSurface()->drawableContentRect());

    // All layers that draw content into renderSurface2 think they are unclipped.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child2->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child3->visibleContentRect());

    // drawableContentRects are also unclipped.
    EXPECT_RECT_EQ(gfx::Rect(5, 5, 50, 50), child1->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(75, 75, 50, 50), child2->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(125, 125, 50, 50), child3->drawableContentRect());
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsWithTransformOnUnclippedSurface)
{
    // Layers that have non-axis aligned bounds (due to transforms) have an expanded,
    // axis-aligned drawableContentRect and visibleContentRect.

    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(renderSurface1);
    renderSurface1->addChild(child1);

    WebTransformationMatrix identityMatrix;
    WebTransformationMatrix childRotation;
    childRotation.rotate(45);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(3, 4), false);
    setLayerPropertiesForTesting(child1.get(), childRotation, identityMatrix, gfx::PointF(0.5, 0.5), gfx::PointF(25, 25), gfx::Size(50, 50), false);

    renderSurface1->setForceRenderSurface(true);
    executeCalculateDrawTransformsAndVisibility(root.get());

    ASSERT_TRUE(renderSurface1->renderSurface());

    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->renderSurface()->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), root->drawableContentRect());

    // Layers that do not draw content should have empty visibleContentRects.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), root->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), renderSurface1->visibleContentRect());

    // The unclipped surface grows its drawableContentRect to include all drawable regions of the subtree.
    int diagonalRadius = ceil(sqrt(2.0) * 25);
    gfx::Rect expectedSurfaceDrawableContent = gfx::Rect(50 - diagonalRadius, 50 - diagonalRadius, diagonalRadius * 2, diagonalRadius * 2);
    EXPECT_RECT_EQ(expectedSurfaceDrawableContent, renderSurface1->renderSurface()->drawableContentRect());

    // All layers that draw content into the unclipped surface are also unclipped.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 50, 50), child1->visibleContentRect());
    EXPECT_RECT_EQ(expectedSurfaceDrawableContent, child1->drawableContentRect());
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsWithTransformOnClippedSurface)
{
    // Layers that have non-axis aligned bounds (due to transforms) have an expanded,
    // axis-aligned drawableContentRect and visibleContentRect.

    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> renderSurface1 = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    root->addChild(renderSurface1);
    renderSurface1->addChild(child1);

    WebTransformationMatrix identityMatrix;
    WebTransformationMatrix childRotation;
    childRotation.rotate(45);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(3, 4), false);
    setLayerPropertiesForTesting(child1.get(), childRotation, identityMatrix, gfx::PointF(0.5, 0.5), gfx::PointF(25, 25), gfx::Size(50, 50), false);

    root->setMasksToBounds(true);
    renderSurface1->setForceRenderSurface(true);
    executeCalculateDrawTransformsAndVisibility(root.get());

    ASSERT_TRUE(renderSurface1->renderSurface());

    // The clipped surface clamps the drawableContentRect that encloses the rotated layer.
    int diagonalRadius = ceil(sqrt(2.0) * 25);
    gfx::Rect unclippedSurfaceContent = gfx::Rect(50 - diagonalRadius, 50 - diagonalRadius, diagonalRadius * 2, diagonalRadius * 2);
    gfx::Rect expectedSurfaceDrawableContent = gfx::IntersectRects(unclippedSurfaceContent, gfx::Rect(0, 0, 50, 50));
    EXPECT_RECT_EQ(expectedSurfaceDrawableContent, renderSurface1->renderSurface()->drawableContentRect());

    // On the clipped surface, only a quarter  of the child1 is visible, but when rotating
    // it back to  child1's content space, the actual enclosing rect ends up covering the
    // full left half of child1.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 26, 50), child1->visibleContentRect());

    // The child's drawableContentRect is unclipped.
    EXPECT_RECT_EQ(unclippedSurfaceContent, child1->drawableContentRect());
}

TEST(LayerTreeHostCommonTest, verifyDrawableAndVisibleContentRectsInHighDPI)
{
    MockContentLayerClient client;

    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<ContentLayer> renderSurface1 = createDrawableContentLayer(&client);
    scoped_refptr<ContentLayer> renderSurface2 = createDrawableContentLayer(&client);
    scoped_refptr<ContentLayer> child1 = createDrawableContentLayer(&client);
    scoped_refptr<ContentLayer> child2 = createDrawableContentLayer(&client);
    scoped_refptr<ContentLayer> child3 = createDrawableContentLayer(&client);
    root->addChild(renderSurface1);
    renderSurface1->addChild(renderSurface2);
    renderSurface2->addChild(child1);
    renderSurface2->addChild(child2);
    renderSurface2->addChild(child3);

    WebTransformationMatrix identityMatrix;
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(renderSurface1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(3, 4), false);
    setLayerPropertiesForTesting(renderSurface2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(7, 13), false);
    setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(5, 5), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(75, 75), gfx::Size(50, 50), false);
    setLayerPropertiesForTesting(child3.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(125, 125), gfx::Size(50, 50), false);

    const double deviceScaleFactor = 2;
    root->setContentsScale(deviceScaleFactor);
    renderSurface1->setContentsScale(deviceScaleFactor);
    renderSurface2->setContentsScale(deviceScaleFactor);
    child1->setContentsScale(deviceScaleFactor);
    child2->setContentsScale(deviceScaleFactor);
    child3->setContentsScale(deviceScaleFactor);

    root->setMasksToBounds(true);
    renderSurface1->setForceRenderSurface(true);
    renderSurface2->setForceRenderSurface(true);
    executeCalculateDrawTransformsAndVisibility(root.get(), deviceScaleFactor);

    ASSERT_TRUE(renderSurface1->renderSurface());
    ASSERT_TRUE(renderSurface2->renderSurface());

    // DrawableContentRects for all layers and surfaces are scaled by deviceScaleFactor.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 200, 200), root->renderSurface()->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 200, 200), root->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(10, 10, 190, 190), renderSurface1->renderSurface()->drawableContentRect());

    // renderSurface2 lives in the "unclipped universe" of renderSurface1, and
    // is only implicitly clipped by renderSurface1.
    EXPECT_RECT_EQ(gfx::Rect(10, 10, 350, 350), renderSurface2->renderSurface()->drawableContentRect());

    EXPECT_RECT_EQ(gfx::Rect(10, 10, 100, 100), child1->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(150, 150, 100, 100), child2->drawableContentRect());
    EXPECT_RECT_EQ(gfx::Rect(250, 250, 100, 100), child3->drawableContentRect());

    // The root layer does not actually draw content of its own.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 0, 0), root->visibleContentRect());

    // All layer visibleContentRects are expressed in content space of each
    // layer, so they are also scaled by the deviceScaleFactor.
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 6, 8), renderSurface1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 14, 26), renderSurface2->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), child1->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), child2->visibleContentRect());
    EXPECT_RECT_EQ(gfx::Rect(0, 0, 100, 100), child3->visibleContentRect());
}

TEST(LayerTreeHostCommonTest, verifyBackFaceCullingWithoutPreserves3d)
{
    // Verify the behavior of back-face culling when there are no preserve-3d layers. Note
    // that 3d transforms still apply in this case, but they are "flattened" to each
    // parent layer according to current W3C spec.

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingChild = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingChild = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingChildOfFrontFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingChildOfFrontFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingChildOfBackFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingChildOfBackFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());

    parent->addChild(frontFacingChild);
    parent->addChild(backFacingChild);
    parent->addChild(frontFacingSurface);
    parent->addChild(backFacingSurface);
    frontFacingSurface->addChild(frontFacingChildOfFrontFacingSurface);
    frontFacingSurface->addChild(backFacingChildOfFrontFacingSurface);
    backFacingSurface->addChild(frontFacingChildOfBackFacingSurface);
    backFacingSurface->addChild(backFacingChildOfBackFacingSurface);

    // Nothing is double-sided
    frontFacingChild->setDoubleSided(false);
    backFacingChild->setDoubleSided(false);
    frontFacingSurface->setDoubleSided(false);
    backFacingSurface->setDoubleSided(false);
    frontFacingChildOfFrontFacingSurface->setDoubleSided(false);
    backFacingChildOfFrontFacingSurface->setDoubleSided(false);
    frontFacingChildOfBackFacingSurface->setDoubleSided(false);
    backFacingChildOfBackFacingSurface->setDoubleSided(false);

    WebTransformationMatrix backfaceMatrix;
    backfaceMatrix.translate(50, 50);
    backfaceMatrix.rotate3d(0, 1, 0, 180);
    backfaceMatrix.translate(-50, -50);

    // Having a descendant and opacity will force these to have render surfaces.
    frontFacingSurface->setOpacity(0.5);
    backFacingSurface->setOpacity(0.5);

    // Nothing preserves 3d. According to current W3C CSS Transforms spec, these layers
    // should blindly use their own local transforms to determine back-face culling.
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(frontFacingChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(backFacingChild.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(frontFacingSurface.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(backFacingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(frontFacingChildOfFrontFacingSurface.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(backFacingChildOfFrontFacingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(frontFacingChildOfBackFacingSurface.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(backFacingChildOfBackFacingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // Verify which renderSurfaces were created.
    EXPECT_FALSE(frontFacingChild->renderSurface());
    EXPECT_FALSE(backFacingChild->renderSurface());
    EXPECT_TRUE(frontFacingSurface->renderSurface());
    EXPECT_TRUE(backFacingSurface->renderSurface());
    EXPECT_FALSE(frontFacingChildOfFrontFacingSurface->renderSurface());
    EXPECT_FALSE(backFacingChildOfFrontFacingSurface->renderSurface());
    EXPECT_FALSE(frontFacingChildOfBackFacingSurface->renderSurface());
    EXPECT_FALSE(backFacingChildOfBackFacingSurface->renderSurface());

    // Verify the renderSurfaceLayerList.
    ASSERT_EQ(3u, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[1]->id());
    // Even though the back facing surface LAYER gets culled, the other descendants should still be added, so the SURFACE should not be culled.
    EXPECT_EQ(backFacingSurface->id(), renderSurfaceLayerList[2]->id());

    // Verify root surface's layerList.
    ASSERT_EQ(3u, renderSurfaceLayerList[0]->renderSurface()->layerList().size());
    EXPECT_EQ(frontFacingChild->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[0]->id());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[1]->id());
    EXPECT_EQ(backFacingSurface->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[2]->id());

    // Verify frontFacingSurface's layerList.
    ASSERT_EQ(2u, renderSurfaceLayerList[1]->renderSurface()->layerList().size());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[0]->id());
    EXPECT_EQ(frontFacingChildOfFrontFacingSurface->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[1]->id());

    // Verify backFacingSurface's layerList; its own layer should be culled from the surface list.
    ASSERT_EQ(1u, renderSurfaceLayerList[2]->renderSurface()->layerList().size());
    EXPECT_EQ(frontFacingChildOfBackFacingSurface->id(), renderSurfaceLayerList[2]->renderSurface()->layerList()[0]->id());
}

TEST(LayerTreeHostCommonTest, verifyBackFaceCullingWithPreserves3d)
{
    // Verify the behavior of back-face culling when preserves-3d transform style is used.

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingChild = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingChild = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingChildOfFrontFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingChildOfFrontFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingChildOfBackFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingChildOfBackFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> dummyReplicaLayer1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> dummyReplicaLayer2 = make_scoped_refptr(new LayerWithForcedDrawsContent());

    parent->addChild(frontFacingChild);
    parent->addChild(backFacingChild);
    parent->addChild(frontFacingSurface);
    parent->addChild(backFacingSurface);
    frontFacingSurface->addChild(frontFacingChildOfFrontFacingSurface);
    frontFacingSurface->addChild(backFacingChildOfFrontFacingSurface);
    backFacingSurface->addChild(frontFacingChildOfBackFacingSurface);
    backFacingSurface->addChild(backFacingChildOfBackFacingSurface);

    // Nothing is double-sided
    frontFacingChild->setDoubleSided(false);
    backFacingChild->setDoubleSided(false);
    frontFacingSurface->setDoubleSided(false);
    backFacingSurface->setDoubleSided(false);
    frontFacingChildOfFrontFacingSurface->setDoubleSided(false);
    backFacingChildOfFrontFacingSurface->setDoubleSided(false);
    frontFacingChildOfBackFacingSurface->setDoubleSided(false);
    backFacingChildOfBackFacingSurface->setDoubleSided(false);

    WebTransformationMatrix backfaceMatrix;
    backfaceMatrix.translate(50, 50);
    backfaceMatrix.rotate3d(0, 1, 0, 180);
    backfaceMatrix.translate(-50, -50);

    // Opacity will not force creation of renderSurfaces in this case because of the
    // preserve-3d transform style. Instead, an example of when a surface would be
    // created with preserve-3d is when there is a replica layer.
    frontFacingSurface->setReplicaLayer(dummyReplicaLayer1.get());
    backFacingSurface->setReplicaLayer(dummyReplicaLayer2.get());

    // Each surface creates its own new 3d rendering context (as defined by W3C spec).
    // According to current W3C CSS Transforms spec, layers in a 3d rendering context
    // should use the transform with respect to that context. This 3d rendering context
    // occurs when (a) parent's transform style is flat and (b) the layer's transform
    // style is preserve-3d.
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false); // parent transform style is flat.
    setLayerPropertiesForTesting(frontFacingChild.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(backFacingChild.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(frontFacingSurface.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), true); // surface transform style is preserve-3d.
    setLayerPropertiesForTesting(backFacingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), true); // surface transform style is preserve-3d.
    setLayerPropertiesForTesting(frontFacingChildOfFrontFacingSurface.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(backFacingChildOfFrontFacingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(frontFacingChildOfBackFacingSurface.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(backFacingChildOfBackFacingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // Verify which renderSurfaces were created.
    EXPECT_FALSE(frontFacingChild->renderSurface());
    EXPECT_FALSE(backFacingChild->renderSurface());
    EXPECT_TRUE(frontFacingSurface->renderSurface());
    EXPECT_FALSE(backFacingSurface->renderSurface());
    EXPECT_FALSE(frontFacingChildOfFrontFacingSurface->renderSurface());
    EXPECT_FALSE(backFacingChildOfFrontFacingSurface->renderSurface());
    EXPECT_FALSE(frontFacingChildOfBackFacingSurface->renderSurface());
    EXPECT_FALSE(backFacingChildOfBackFacingSurface->renderSurface());

    // Verify the renderSurfaceLayerList. The back-facing surface should be culled.
    ASSERT_EQ(2u, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[1]->id());

    // Verify root surface's layerList.
    ASSERT_EQ(2u, renderSurfaceLayerList[0]->renderSurface()->layerList().size());
    EXPECT_EQ(frontFacingChild->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[0]->id());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[1]->id());

    // Verify frontFacingSurface's layerList.
    ASSERT_EQ(2u, renderSurfaceLayerList[1]->renderSurface()->layerList().size());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[0]->id());
    EXPECT_EQ(frontFacingChildOfFrontFacingSurface->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[1]->id());
}

TEST(LayerTreeHostCommonTest, verifyBackFaceCullingWithAnimatingTransforms)
{
    // Verify that layers are appropriately culled when their back face is showing and
    // they are not double sided, while animations are going on.
    //
    // Layers that are animating do not get culled on the main thread, as their transforms should be
    // treated as "unknown" so we can not be sure that their back face is really showing.
    //

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> child = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> animatingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> childOfAnimatingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> animatingChild = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child2 = make_scoped_refptr(new LayerWithForcedDrawsContent());

    parent->addChild(child);
    parent->addChild(animatingSurface);
    animatingSurface->addChild(childOfAnimatingSurface);
    parent->addChild(animatingChild);
    parent->addChild(child2);

    // Nothing is double-sided
    child->setDoubleSided(false);
    child2->setDoubleSided(false);
    animatingSurface->setDoubleSided(false);
    childOfAnimatingSurface->setDoubleSided(false);
    animatingChild->setDoubleSided(false);

    WebTransformationMatrix backfaceMatrix;
    backfaceMatrix.translate(50, 50);
    backfaceMatrix.rotate3d(0, 1, 0, 180);
    backfaceMatrix.translate(-50, -50);

    // Make our render surface.
    animatingSurface->setForceRenderSurface(true);

    // Animate the transform on the render surface.
    addAnimatedTransformToController(*animatingSurface->layerAnimationController(), 10, 30, 0);
    // This is just an animating layer, not a surface.
    addAnimatedTransformToController(*animatingChild->layerAnimationController(), 10, 30, 0);

    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(animatingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(childOfAnimatingSurface.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(animatingChild.get(), backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_FALSE(child->renderSurface());
    EXPECT_TRUE(animatingSurface->renderSurface());
    EXPECT_FALSE(childOfAnimatingSurface->renderSurface());
    EXPECT_FALSE(animatingChild->renderSurface());
    EXPECT_FALSE(child2->renderSurface());

    // Verify that the animatingChild and childOfAnimatingSurface were not culled, but that child was.
    ASSERT_EQ(2u, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());
    EXPECT_EQ(animatingSurface->id(), renderSurfaceLayerList[1]->id());

    // The non-animating child be culled from the layer list for the parent render surface.
    ASSERT_EQ(3u, renderSurfaceLayerList[0]->renderSurface()->layerList().size());
    EXPECT_EQ(animatingSurface->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[0]->id());
    EXPECT_EQ(animatingChild->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[1]->id());
    EXPECT_EQ(child2->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[2]->id());

    ASSERT_EQ(2u, renderSurfaceLayerList[1]->renderSurface()->layerList().size());
    EXPECT_EQ(animatingSurface->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[0]->id());
    EXPECT_EQ(childOfAnimatingSurface->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[1]->id());

    EXPECT_FALSE(child2->visibleContentRect().IsEmpty());

    // The animating layers should have a visibleContentRect that represents the area of the front face that is within the viewport.
    EXPECT_EQ(animatingChild->visibleContentRect(), gfx::Rect(gfx::Point(), animatingChild->contentBounds()));
    EXPECT_EQ(animatingSurface->visibleContentRect(), gfx::Rect(gfx::Point(), animatingSurface->contentBounds()));
    // And layers in the subtree of the animating layer should have valid visibleContentRects also.
    EXPECT_EQ(childOfAnimatingSurface->visibleContentRect(), gfx::Rect(gfx::Point(), childOfAnimatingSurface->contentBounds()));
}

TEST(LayerTreeHostCommonTest, verifyBackFaceCullingWithPreserves3dForFlatteningSurface)
{
    // Verify the behavior of back-face culling for a renderSurface that is created
    // when it flattens its subtree, and its parent has preserves-3d.

    const WebTransformationMatrix identityMatrix;
    scoped_refptr<Layer> parent = Layer::create();
    scoped_refptr<LayerWithForcedDrawsContent> frontFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> backFacingSurface = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child1 = make_scoped_refptr(new LayerWithForcedDrawsContent());
    scoped_refptr<LayerWithForcedDrawsContent> child2 = make_scoped_refptr(new LayerWithForcedDrawsContent());

    parent->addChild(frontFacingSurface);
    parent->addChild(backFacingSurface);
    frontFacingSurface->addChild(child1);
    backFacingSurface->addChild(child2);

    // RenderSurfaces are not double-sided
    frontFacingSurface->setDoubleSided(false);
    backFacingSurface->setDoubleSided(false);

    WebTransformationMatrix backfaceMatrix;
    backfaceMatrix.translate(50, 50);
    backfaceMatrix.rotate3d(0, 1, 0, 180);
    backfaceMatrix.translate(-50, -50);

    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), true); // parent transform style is preserve3d.
    setLayerPropertiesForTesting(frontFacingSurface.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false); // surface transform style is flat.
    setLayerPropertiesForTesting(backFacingSurface.get(),  backfaceMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false); // surface transform style is flat.
    setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);
    setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), false);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), 1, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // Verify which renderSurfaces were created.
    EXPECT_TRUE(frontFacingSurface->renderSurface());
    EXPECT_FALSE(backFacingSurface->renderSurface()); // because it should be culled
    EXPECT_FALSE(child1->renderSurface());
    EXPECT_FALSE(child2->renderSurface());

    // Verify the renderSurfaceLayerList. The back-facing surface should be culled.
    ASSERT_EQ(2u, renderSurfaceLayerList.size());
    EXPECT_EQ(parent->id(), renderSurfaceLayerList[0]->id());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[1]->id());

    // Verify root surface's layerList.
    ASSERT_EQ(1u, renderSurfaceLayerList[0]->renderSurface()->layerList().size());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[0]->renderSurface()->layerList()[0]->id());

    // Verify frontFacingSurface's layerList.
    ASSERT_EQ(2u, renderSurfaceLayerList[1]->renderSurface()->layerList().size());
    EXPECT_EQ(frontFacingSurface->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[0]->id());
    EXPECT_EQ(child1->id(), renderSurfaceLayerList[1]->renderSurface()->layerList()[1]->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForEmptyLayerList)
{
    // Hit testing on an empty renderSurfaceLayerList should return a null pointer.
    std::vector<LayerImpl*> renderSurfaceLayerList;

    gfx::Point testPoint(0, 0);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(10, 20);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForSingleLayer)
{
    scoped_ptr<LayerImpl> root = LayerImpl::create(12345);

    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
    root->setDrawsContent(true);

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());

    // Hit testing for a point outside the layer should return a null pointer.
    gfx::Point testPoint(101, 101);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(-1, -1);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Hit testing for a point inside should return the root layer.
    testPoint = gfx::Point(1, 1);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());

    testPoint = gfx::Point(99, 99);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForUninvertibleTransform)
{
    scoped_ptr<LayerImpl> root = LayerImpl::create(12345);

    WebTransformationMatrix uninvertibleTransform;
    uninvertibleTransform.setM11(0);
    uninvertibleTransform.setM22(0);
    uninvertibleTransform.setM33(0);
    uninvertibleTransform.setM44(0);
    ASSERT_FALSE(uninvertibleTransform.isInvertible());

    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), uninvertibleTransform, identityMatrix, anchor, position, bounds, false);
    root->setDrawsContent(true);

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());
    ASSERT_FALSE(root->screenSpaceTransform().isInvertible());

    // Hit testing any point should not hit the layer. If the invertible matrix is
    // accidentally ignored and treated like an identity, then the hit testing will
    // incorrectly hit the layer when it shouldn't.
    gfx::Point testPoint(1, 1);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(10, 10);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(10, 30);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(50, 50);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(67, 48);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(99, 99);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(-1, -1);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForSinglePositionedLayer)
{
    scoped_ptr<LayerImpl> root = LayerImpl::create(12345);

    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);
    gfx::PointF position(50, 50); // this layer is positioned, and hit testing should correctly know where the layer is located.
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
    root->setDrawsContent(true);

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());

    // Hit testing for a point outside the layer should return a null pointer.
    gfx::Point testPoint(49, 49);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Even though the layer exists at (101, 101), it should not be visible there since the root renderSurface would clamp it.
    testPoint = gfx::Point(101, 101);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Hit testing for a point inside should return the root layer.
    testPoint = gfx::Point(51, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());

    testPoint = gfx::Point(99, 99);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForSingleRotatedLayer)
{
    scoped_ptr<LayerImpl> root = LayerImpl::create(12345);

    WebTransformationMatrix identityMatrix;
    WebTransformationMatrix rotation45DegreesAboutCenter;
    rotation45DegreesAboutCenter.translate(50, 50);
    rotation45DegreesAboutCenter.rotate3d(0, 0, 45);
    rotation45DegreesAboutCenter.translate(-50, -50);
    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), rotation45DegreesAboutCenter, identityMatrix, anchor, position, bounds, false);
    root->setDrawsContent(true);

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());

    // Hit testing for points outside the layer.
    // These corners would have been inside the un-transformed layer, but they should not hit the correctly transformed layer.
    gfx::Point testPoint(99, 99);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(1, 1);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Hit testing for a point inside should return the root layer.
    testPoint = gfx::Point(1, 50);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());

    // Hit testing the corners that would overlap the unclipped layer, but are outside the clipped region.
    testPoint = gfx::Point(50, -1);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_FALSE(resultLayer);

    testPoint = gfx::Point(-1, 50);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_FALSE(resultLayer);
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForSinglePerspectiveLayer)
{
    scoped_ptr<LayerImpl> root = LayerImpl::create(12345);

    WebTransformationMatrix identityMatrix;

    // perspectiveProjectionAboutCenter * translationByZ is designed so that the 100 x 100 layer becomes 50 x 50, and remains centered at (50, 50).
    WebTransformationMatrix perspectiveProjectionAboutCenter;
    perspectiveProjectionAboutCenter.translate(50, 50);
    perspectiveProjectionAboutCenter.applyPerspective(1);
    perspectiveProjectionAboutCenter.translate(-50, -50);
    WebTransformationMatrix translationByZ;
    translationByZ.translate3d(0, 0, -1);

    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), perspectiveProjectionAboutCenter * translationByZ, identityMatrix, anchor, position, bounds, false);
    root->setDrawsContent(true);

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());

    // Hit testing for points outside the layer.
    // These corners would have been inside the un-transformed layer, but they should not hit the correctly transformed layer.
    gfx::Point testPoint(24, 24);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(76, 76);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Hit testing for a point inside should return the root layer.
    testPoint = gfx::Point(26, 26);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());

    testPoint = gfx::Point(74, 74);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForSingleLayerWithScaledContents)
{
    // A layer's visibleContentRect is actually in the layer's content space. The
    // screenSpaceTransform converts from the layer's origin space to screen space. This
    // test makes sure that hit testing works correctly accounts for the contents scale.
    // A contentsScale that is not 1 effectively forces a non-identity transform between
    // layer's content space and layer's origin space. The hit testing code must take this into account.
    //
    // To test this, the layer is positioned at (25, 25), and is size (50, 50). If
    // contentsScale is ignored, then hit testing will mis-interpret the visibleContentRect
    // as being larger than the actual bounds of the layer.
    //
    scoped_ptr<LayerImpl> root = LayerImpl::create(1);

    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);

    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, gfx::PointF(0, 0), gfx::Size(100, 100), false);

    {
        gfx::PointF position(25, 25);
        gfx::Size bounds(50, 50);
        scoped_ptr<LayerImpl> testLayer = LayerImpl::create(12345);
        setLayerPropertiesForTesting(testLayer.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);

        // override contentBounds and contentsScale
        testLayer->setContentBounds(gfx::Size(100, 100));
        testLayer->setContentsScale(2, 2);

        testLayer->setDrawsContent(true);
        root->addChild(testLayer.Pass());
    }

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    // The visibleContentRect for testLayer is actually 100x100, even though its layout size is 50x50, positioned at 25x25.
    LayerImpl* testLayer = root->children()[0];
    EXPECT_RECT_EQ(gfx::Rect(gfx::Point(), gfx::Size(100, 100)), testLayer->visibleContentRect());
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());

    // Hit testing for a point outside the layer should return a null pointer (the root layer does not draw content, so it will not be hit tested either).
    gfx::Point testPoint(101, 101);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(24, 24);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(76, 76);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Hit testing for a point inside should return the test layer.
    testPoint = gfx::Point(26, 26);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());

    testPoint = gfx::Point(74, 74);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(12345, resultLayer->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForSimpleClippedLayer)
{
    // Test that hit-testing will only work for the visible portion of a layer, and not
    // the entire layer bounds. Here we just test the simple axis-aligned case.
    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);

    scoped_ptr<LayerImpl> root = LayerImpl::create(1);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, gfx::PointF(0, 0), gfx::Size(100, 100), false);

    {
        scoped_ptr<LayerImpl> clippingLayer = LayerImpl::create(123);
        gfx::PointF position(25, 25); // this layer is positioned, and hit testing should correctly know where the layer is located.
        gfx::Size bounds(50, 50);
        setLayerPropertiesForTesting(clippingLayer.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        clippingLayer->setMasksToBounds(true);

        scoped_ptr<LayerImpl> child = LayerImpl::create(456);
        position = gfx::PointF(-50, -50);
        bounds = gfx::Size(300, 300);
        setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        child->setDrawsContent(true);
        clippingLayer->addChild(child.Pass());
        root->addChild(clippingLayer.Pass());
    }

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());
    ASSERT_EQ(456, root->renderSurface()->layerList()[0]->id());

    // Hit testing for a point outside the layer should return a null pointer.
    // Despite the child layer being very large, it should be clipped to the root layer's bounds.
    gfx::Point testPoint(24, 24);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Even though the layer exists at (101, 101), it should not be visible there since the clippingLayer would clamp it.
    testPoint = gfx::Point(76, 76);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Hit testing for a point inside should return the child layer.
    testPoint = gfx::Point(26, 26);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(456, resultLayer->id());

    testPoint = gfx::Point(74, 74);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(456, resultLayer->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForMultiClippedRotatedLayer)
{
    // This test checks whether hit testing correctly avoids hit testing with multiple
    // ancestors that clip in non axis-aligned ways. To pass this test, the hit testing
    // algorithm needs to recognize that multiple parent layers may clip the layer, and
    // should not actually hit those clipped areas.
    //
    // The child and grandChild layers are both initialized to clip the rotatedLeaf. The
    // child layer is rotated about the top-left corner, so that the root + child clips
    // combined create a triangle. The rotatedLeaf will only be visible where it overlaps
    // this triangle.
    //
    scoped_ptr<LayerImpl> root = LayerImpl::create(123);

    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
    root->setMasksToBounds(true);

    {
        scoped_ptr<LayerImpl> child = LayerImpl::create(456);
        scoped_ptr<LayerImpl> grandChild = LayerImpl::create(789);
        scoped_ptr<LayerImpl> rotatedLeaf = LayerImpl::create(2468);

        position = gfx::PointF(10, 10);
        bounds = gfx::Size(80, 80);
        setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        child->setMasksToBounds(true);
        
        WebTransformationMatrix rotation45DegreesAboutCorner;
        rotation45DegreesAboutCorner.rotate3d(0, 0, 45);

        position = gfx::PointF(0, 0); // remember, positioned with respect to its parent which is already at 10, 10
        bounds = gfx::Size(200, 200); // to ensure it covers at least sqrt(2) * 100.
        setLayerPropertiesForTesting(grandChild.get(), rotation45DegreesAboutCorner, identityMatrix, anchor, position, bounds, false);
        grandChild->setMasksToBounds(true);

        // Rotates about the center of the layer
        WebTransformationMatrix rotatedLeafTransform;
        rotatedLeafTransform.translate(-10, -10); // cancel out the grandParent's position
        rotatedLeafTransform.rotate3d(0, 0, -45); // cancel out the corner 45-degree rotation of the parent.
        rotatedLeafTransform.translate(50, 50);
        rotatedLeafTransform.rotate3d(0, 0, 45);
        rotatedLeafTransform.translate(-50, -50);
        position = gfx::PointF(0, 0);
        bounds = gfx::Size(100, 100);
        setLayerPropertiesForTesting(rotatedLeaf.get(), rotatedLeafTransform, identityMatrix, anchor, position, bounds, false);
        rotatedLeaf->setDrawsContent(true);

        grandChild->addChild(rotatedLeaf.Pass());
        child->addChild(grandChild.Pass());
        root->addChild(child.Pass());
    }

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    // The grandChild is expected to create a renderSurface because it masksToBounds and is not axis aligned.
    ASSERT_EQ(2u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, renderSurfaceLayerList[0]->renderSurface()->layerList().size());
    ASSERT_EQ(789, renderSurfaceLayerList[0]->renderSurface()->layerList()[0]->id()); // grandChild's surface.
    ASSERT_EQ(1u, renderSurfaceLayerList[1]->renderSurface()->layerList().size());
    ASSERT_EQ(2468, renderSurfaceLayerList[1]->renderSurface()->layerList()[0]->id());

    // (11, 89) is close to the the bottom left corner within the clip, but it is not inside the layer.
    gfx::Point testPoint(11, 89);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Closer inwards from the bottom left will overlap the layer.
    testPoint = gfx::Point(25, 75);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(2468, resultLayer->id());

    // (4, 50) is inside the unclipped layer, but that corner of the layer should be
    // clipped away by the grandParent and should not get hit. If hit testing blindly uses
    // visibleContentRect without considering how parent may clip the layer, then hit
    // testing would accidentally think that the point successfully hits the layer.
    testPoint = gfx::Point(4, 50);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // (11, 50) is inside the layer and within the clipped area.
    testPoint = gfx::Point(11, 50);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(2468, resultLayer->id());

    // Around the middle, just to the right and up, would have hit the layer except that
    // that area should be clipped away by the parent.
    testPoint = gfx::Point(51, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Around the middle, just to the left and down, should successfully hit the layer.
    testPoint = gfx::Point(49, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(2468, resultLayer->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForNonClippingIntermediateLayer)
{
    // This test checks that hit testing code does not accidentally clip to layer
    // bounds for a layer that actually does not clip.
    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);

    scoped_ptr<LayerImpl> root = LayerImpl::create(1);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, gfx::PointF(0, 0), gfx::Size(100, 100), false);

    {
        scoped_ptr<LayerImpl> intermediateLayer = LayerImpl::create(123);
        gfx::PointF position(10, 10); // this layer is positioned, and hit testing should correctly know where the layer is located.
        gfx::Size bounds(50, 50);
        setLayerPropertiesForTesting(intermediateLayer.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        // Sanity check the intermediate layer should not clip.
        ASSERT_FALSE(intermediateLayer->masksToBounds());
        ASSERT_FALSE(intermediateLayer->maskLayer());

        // The child of the intermediateLayer is translated so that it does not overlap intermediateLayer at all.
        // If child is incorrectly clipped, we would not be able to hit it successfully.
        scoped_ptr<LayerImpl> child = LayerImpl::create(456);
        position = gfx::PointF(60, 60); // 70, 70 in screen space
        bounds = gfx::Size(20, 20);
        setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        child->setDrawsContent(true);
        intermediateLayer->addChild(child.Pass());
        root->addChild(intermediateLayer.Pass());
    }

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(1u, root->renderSurface()->layerList().size());
    ASSERT_EQ(456, root->renderSurface()->layerList()[0]->id());

    // Hit testing for a point outside the layer should return a null pointer.
    gfx::Point testPoint(69, 69);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    testPoint = gfx::Point(91, 91);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    EXPECT_FALSE(resultLayer);

    // Hit testing for a point inside should return the child layer.
    testPoint = gfx::Point(71, 71);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(456, resultLayer->id());

    testPoint = gfx::Point(89, 89);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(456, resultLayer->id());
}


TEST(LayerTreeHostCommonTest, verifyHitTestingForMultipleLayers)
{
    scoped_ptr<LayerImpl> root = LayerImpl::create(1);

    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
    root->setDrawsContent(true);

    {
        // child 1 and child2 are initialized to overlap between x=50 and x=60.
        // grandChild is set to overlap both child1 and child2 between y=50 and y=60.
        // The expected stacking order is:
        //   (front) child2, (second) grandChild, (third) child1, and (back) the root layer behind all other layers.

        scoped_ptr<LayerImpl> child1 = LayerImpl::create(2);
        scoped_ptr<LayerImpl> child2 = LayerImpl::create(3);
        scoped_ptr<LayerImpl> grandChild1 = LayerImpl::create(4);

        position = gfx::PointF(10, 10);
        bounds = gfx::Size(50, 50);
        setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        child1->setDrawsContent(true);

        position = gfx::PointF(50, 10);
        bounds = gfx::Size(50, 50);
        setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        child2->setDrawsContent(true);

        // Remember that grandChild is positioned with respect to its parent (i.e. child1).
        // In screen space, the intended position is (10, 50), with size 100 x 50.
        position = gfx::PointF(0, 40);
        bounds = gfx::Size(100, 50);
        setLayerPropertiesForTesting(grandChild1.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        grandChild1->setDrawsContent(true);

        child1->addChild(grandChild1.Pass());
        root->addChild(child1.Pass());
        root->addChild(child2.Pass());
    }

    LayerImpl* child1 = root->children()[0];
    LayerImpl* child2 = root->children()[1];
    LayerImpl* grandChild1 = child1->children()[0];

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_TRUE(child1);
    ASSERT_TRUE(child2);
    ASSERT_TRUE(grandChild1);
    ASSERT_EQ(1u, renderSurfaceLayerList.size());
    ASSERT_EQ(4u, root->renderSurface()->layerList().size());
    ASSERT_EQ(1, root->renderSurface()->layerList()[0]->id()); // root layer
    ASSERT_EQ(2, root->renderSurface()->layerList()[1]->id()); // child1
    ASSERT_EQ(4, root->renderSurface()->layerList()[2]->id()); // grandChild1
    ASSERT_EQ(3, root->renderSurface()->layerList()[3]->id()); // child2

    // Nothing overlaps the rootLayer at (1, 1), so hit testing there should find the root layer.
    gfx::Point testPoint = gfx::Point(1, 1);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(1, resultLayer->id());

    // At (15, 15), child1 and root are the only layers. child1 is expected to be on top.
    testPoint = gfx::Point(15, 15);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(2, resultLayer->id());

    // At (51, 20), child1 and child2 overlap. child2 is expected to be on top.
    testPoint = gfx::Point(51, 20);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(3, resultLayer->id());

    // At (80, 51), child2 and grandChild1 overlap. child2 is expected to be on top.
    testPoint = gfx::Point(80, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(3, resultLayer->id());

    // At (51, 51), all layers overlap each other. child2 is expected to be on top of all other layers.
    testPoint = gfx::Point(51, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(3, resultLayer->id());

    // At (20, 51), child1 and grandChild1 overlap. grandChild1 is expected to be on top.
    testPoint = gfx::Point(20, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(4, resultLayer->id());
}

TEST(LayerTreeHostCommonTest, verifyHitTestingForMultipleLayerLists)
{
    //
    // The geometry is set up similarly to the previous case, but
    // all layers are forced to be renderSurfaces now.
    //
    scoped_ptr<LayerImpl> root = LayerImpl::create(1);

    WebTransformationMatrix identityMatrix;
    gfx::PointF anchor(0, 0);
    gfx::PointF position(0, 0);
    gfx::Size bounds(100, 100);
    setLayerPropertiesForTesting(root.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
    root->setDrawsContent(true);

    {
        // child 1 and child2 are initialized to overlap between x=50 and x=60.
        // grandChild is set to overlap both child1 and child2 between y=50 and y=60.
        // The expected stacking order is:
        //   (front) child2, (second) grandChild, (third) child1, and (back) the root layer behind all other layers.

        scoped_ptr<LayerImpl> child1 = LayerImpl::create(2);
        scoped_ptr<LayerImpl> child2 = LayerImpl::create(3);
        scoped_ptr<LayerImpl> grandChild1 = LayerImpl::create(4);

        position = gfx::PointF(10, 10);
        bounds = gfx::Size(50, 50);
        setLayerPropertiesForTesting(child1.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        child1->setDrawsContent(true);
        child1->setForceRenderSurface(true);

        position = gfx::PointF(50, 10);
        bounds = gfx::Size(50, 50);
        setLayerPropertiesForTesting(child2.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        child2->setDrawsContent(true);
        child2->setForceRenderSurface(true);

        // Remember that grandChild is positioned with respect to its parent (i.e. child1).
        // In screen space, the intended position is (10, 50), with size 100 x 50.
        position = gfx::PointF(0, 40);
        bounds = gfx::Size(100, 50);
        setLayerPropertiesForTesting(grandChild1.get(), identityMatrix, identityMatrix, anchor, position, bounds, false);
        grandChild1->setDrawsContent(true);
        grandChild1->setForceRenderSurface(true);

        child1->addChild(grandChild1.Pass());
        root->addChild(child1.Pass());
        root->addChild(child2.Pass());
    }

    LayerImpl* child1 = root->children()[0];
    LayerImpl* child2 = root->children()[1];
    LayerImpl* grandChild1 = child1->children()[0];

    std::vector<LayerImpl*> renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;
    LayerTreeHostCommon::calculateDrawTransforms(root.get(), root->bounds(), 1, 1, 0, dummyMaxTextureSize, renderSurfaceLayerList);

    // Sanity check the scenario we just created.
    ASSERT_TRUE(child1);
    ASSERT_TRUE(child2);
    ASSERT_TRUE(grandChild1);
    ASSERT_TRUE(child1->renderSurface());
    ASSERT_TRUE(child2->renderSurface());
    ASSERT_TRUE(grandChild1->renderSurface());
    ASSERT_EQ(4u, renderSurfaceLayerList.size());
    ASSERT_EQ(3u, root->renderSurface()->layerList().size()); // The root surface has the root layer, and child1's and child2's renderSurfaces.
    ASSERT_EQ(2u, child1->renderSurface()->layerList().size()); // The child1 surface has the child1 layer and grandChild1's renderSurface.
    ASSERT_EQ(1u, child2->renderSurface()->layerList().size());
    ASSERT_EQ(1u, grandChild1->renderSurface()->layerList().size());
    ASSERT_EQ(1, renderSurfaceLayerList[0]->id()); // root layer
    ASSERT_EQ(2, renderSurfaceLayerList[1]->id()); // child1
    ASSERT_EQ(4, renderSurfaceLayerList[2]->id()); // grandChild1
    ASSERT_EQ(3, renderSurfaceLayerList[3]->id()); // child2

    // Nothing overlaps the rootLayer at (1, 1), so hit testing there should find the root layer.
    gfx::Point testPoint = gfx::Point(1, 1);
    LayerImpl* resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(1, resultLayer->id());

    // At (15, 15), child1 and root are the only layers. child1 is expected to be on top.
    testPoint = gfx::Point(15, 15);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(2, resultLayer->id());

    // At (51, 20), child1 and child2 overlap. child2 is expected to be on top.
    testPoint = gfx::Point(51, 20);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(3, resultLayer->id());

    // At (80, 51), child2 and grandChild1 overlap. child2 is expected to be on top.
    testPoint = gfx::Point(80, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(3, resultLayer->id());

    // At (51, 51), all layers overlap each other. child2 is expected to be on top of all other layers.
    testPoint = gfx::Point(51, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(3, resultLayer->id());

    // At (20, 51), child1 and grandChild1 overlap. grandChild1 is expected to be on top.
    testPoint = gfx::Point(20, 51);
    resultLayer = LayerTreeHostCommon::findLayerThatIsHitByPoint(testPoint, renderSurfaceLayerList);
    ASSERT_TRUE(resultLayer);
    EXPECT_EQ(4, resultLayer->id());
}

class NoScaleContentLayer : public ContentLayer
{
public:
    static scoped_refptr<NoScaleContentLayer> create(ContentLayerClient* client) { return make_scoped_refptr(new NoScaleContentLayer(client)); }

    virtual gfx::Size contentBounds() const OVERRIDE { return bounds(); }
    virtual float contentsScaleX() const OVERRIDE { return 1.0; }
    virtual float contentsScaleY() const OVERRIDE { return 1.0; }

protected:
    explicit NoScaleContentLayer(ContentLayerClient* client) : ContentLayer(client) { }
    virtual ~NoScaleContentLayer() { }
};

scoped_refptr<NoScaleContentLayer> createNoScaleDrawableContentLayer(ContentLayerClient* delegate)
{
    scoped_refptr<NoScaleContentLayer> toReturn = NoScaleContentLayer::create(delegate);
    toReturn->setIsDrawable(true);
    return toReturn;
}

TEST(LayerTreeHostCommonTest, verifyLayerTransformsInHighDPI)
{
    // Verify draw and screen space transforms of layers not in a surface.
    MockContentLayerClient delegate;
    WebTransformationMatrix identityMatrix;

    scoped_refptr<ContentLayer> parent = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), true);

    scoped_refptr<ContentLayer> child = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(2, 2), gfx::Size(10, 10), true);

    scoped_refptr<NoScaleContentLayer> childNoScale = createNoScaleDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(childNoScale.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(2, 2), gfx::Size(10, 10), true);

    parent->addChild(child);
    parent->addChild(childNoScale);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;

    const double deviceScaleFactor = 2.5;
    const double pageScaleFactor = 1;

    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, pageScaleFactor, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor, parent);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor, child);
    EXPECT_CONTENTS_SCALE_EQ(1, childNoScale);

    EXPECT_EQ(1u, renderSurfaceLayerList.size());

    // Verify parent transforms
    WebTransformationMatrix expectedParentTransform;
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedParentTransform, parent->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedParentTransform, parent->drawTransform());

    // Verify results of transformed parent rects
    gfx::RectF parentContentBounds(gfx::PointF(), gfx::SizeF(parent->contentBounds()));

    gfx::RectF parentDrawRect = MathUtil::mapClippedRect(parent->drawTransform(), parentContentBounds);
    gfx::RectF parentScreenSpaceRect = MathUtil::mapClippedRect(parent->screenSpaceTransform(), parentContentBounds);

    gfx::RectF expectedParentDrawRect(gfx::PointF(), parent->bounds());
    expectedParentDrawRect.Scale(deviceScaleFactor);
    EXPECT_FLOAT_RECT_EQ(expectedParentDrawRect, parentDrawRect);
    EXPECT_FLOAT_RECT_EQ(expectedParentDrawRect, parentScreenSpaceRect);

    // Verify child transforms
    WebTransformationMatrix expectedChildTransform;
    expectedChildTransform.translate(deviceScaleFactor * child->position().x(), deviceScaleFactor * child->position().y());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->screenSpaceTransform());

    // Verify results of transformed child rects
    gfx::RectF childContentBounds(gfx::PointF(), gfx::SizeF(child->contentBounds()));

    gfx::RectF childDrawRect = MathUtil::mapClippedRect(child->drawTransform(), childContentBounds);
    gfx::RectF childScreenSpaceRect = MathUtil::mapClippedRect(child->screenSpaceTransform(), childContentBounds);

    gfx::RectF expectedChildDrawRect(gfx::PointF(), child->bounds());
    expectedChildDrawRect.Offset(child->position().x(), child->position().y());
    expectedChildDrawRect.Scale(deviceScaleFactor);
    EXPECT_FLOAT_RECT_EQ(expectedChildDrawRect, childDrawRect);
    EXPECT_FLOAT_RECT_EQ(expectedChildDrawRect, childScreenSpaceRect);

    // Verify childNoScale transforms
    WebTransformationMatrix expectedChildNoScaleTransform = child->drawTransform();
    // All transforms operate on content rects. The child's content rect
    // incorporates device scale, but the childNoScale does not; add it here.
    expectedChildNoScaleTransform.scale(deviceScaleFactor);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildNoScaleTransform, childNoScale->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildNoScaleTransform, childNoScale->screenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyLayerTransformsInHighDPIAccurateScaleZeroChildPosition)
{
    // Verify draw and screen space transforms of layers not in a surface.
    MockContentLayerClient delegate;
    WebTransformationMatrix identityMatrix;

    scoped_refptr<ContentLayer> parent = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(133, 133), true);

    scoped_refptr<ContentLayer> child = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(13, 13), true);

    scoped_refptr<NoScaleContentLayer> childNoScale = createNoScaleDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(childNoScale.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(13, 13), true);

    parent->addChild(child);
    parent->addChild(childNoScale);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;

    const float deviceScaleFactor = 1.7f;
    const float pageScaleFactor = 1;

    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, pageScaleFactor, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor, parent);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor, child);
    EXPECT_CONTENTS_SCALE_EQ(1, childNoScale);

    EXPECT_EQ(1u, renderSurfaceLayerList.size());

    // Verify parent transforms
    WebTransformationMatrix expectedParentTransform;
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedParentTransform, parent->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedParentTransform, parent->drawTransform());

    // Verify results of transformed parent rects
    gfx::RectF parentContentBounds(gfx::PointF(), gfx::SizeF(parent->contentBounds()));

    gfx::RectF parentDrawRect = MathUtil::mapClippedRect(parent->drawTransform(), parentContentBounds);
    gfx::RectF parentScreenSpaceRect = MathUtil::mapClippedRect(parent->screenSpaceTransform(), parentContentBounds);

    gfx::RectF expectedParentDrawRect(gfx::PointF(), parent->bounds());
    expectedParentDrawRect.Scale(deviceScaleFactor);
    expectedParentDrawRect.set_width(ceil(expectedParentDrawRect.width()));
    expectedParentDrawRect.set_height(ceil(expectedParentDrawRect.height()));
    EXPECT_FLOAT_RECT_EQ(expectedParentDrawRect, parentDrawRect);
    EXPECT_FLOAT_RECT_EQ(expectedParentDrawRect, parentScreenSpaceRect);

    // Verify child transforms
    WebTransformationMatrix expectedChildTransform;
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildTransform, child->screenSpaceTransform());

    // Verify results of transformed child rects
    gfx::RectF childContentBounds(gfx::PointF(), gfx::SizeF(child->contentBounds()));

    gfx::RectF childDrawRect = MathUtil::mapClippedRect(child->drawTransform(), childContentBounds);
    gfx::RectF childScreenSpaceRect = MathUtil::mapClippedRect(child->screenSpaceTransform(), childContentBounds);

    gfx::RectF expectedChildDrawRect(gfx::PointF(), child->bounds());
    expectedChildDrawRect.Scale(deviceScaleFactor);
    expectedChildDrawRect.set_width(ceil(expectedChildDrawRect.width()));
    expectedChildDrawRect.set_height(ceil(expectedChildDrawRect.height()));
    EXPECT_FLOAT_RECT_EQ(expectedChildDrawRect, childDrawRect);
    EXPECT_FLOAT_RECT_EQ(expectedChildDrawRect, childScreenSpaceRect);

    // Verify childNoScale transforms
    WebTransformationMatrix expectedChildNoScaleTransform = child->drawTransform();
    // All transforms operate on content rects. The child's content rect
    // incorporates device scale, but the childNoScale does not; add it here.
    expectedChildNoScaleTransform.scale(deviceScaleFactor);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildNoScaleTransform, childNoScale->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedChildNoScaleTransform, childNoScale->screenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyContentsScale)
{
    MockContentLayerClient delegate;
    WebTransformationMatrix identityMatrix;

    WebTransformationMatrix parentScaleMatrix;
    const double initialParentScale = 1.75;
    parentScaleMatrix.scale(initialParentScale);

    WebTransformationMatrix childScaleMatrix;
    const double initialChildScale = 1.25;
    childScaleMatrix.scale(initialChildScale);

    float fixedRasterScale = 2.5;

    scoped_refptr<ContentLayer> parent = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(parent.get(), parentScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), true);

    scoped_refptr<ContentLayer> childScale = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(childScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(2, 2), gfx::Size(10, 10), true);

    scoped_refptr<NoScaleContentLayer> childNoScale = createNoScaleDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(childNoScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(12, 12), gfx::Size(10, 10), true);

    scoped_refptr<ContentLayer> childNoAutoScale = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(childNoAutoScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(22, 22), gfx::Size(10, 10), true);
    childNoAutoScale->setAutomaticallyComputeRasterScale(false);
    childNoAutoScale->setRasterScale(fixedRasterScale);

    // FIXME: Remove this when pageScaleFactor is applied in the compositor.
    // Page scale should not apply to the parent.
    parent->setBoundsContainPageScale(true);

    parent->addChild(childScale);
    parent->addChild(childNoScale);
    parent->addChild(childNoAutoScale);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;

    double deviceScaleFactor = 2.5;
    double pageScaleFactor = 1.5;

    // FIXME: Remove this when pageScaleFactor is applied in the compositor.
    WebTransformationMatrix pageScaleMatrix;
    pageScaleMatrix.scale(pageScaleFactor);
    parent->setSublayerTransform(pageScaleMatrix);

    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, pageScaleFactor, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * initialParentScale, parent);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, childScale);
    EXPECT_CONTENTS_SCALE_EQ(1, childNoScale);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * fixedRasterScale, childNoAutoScale);

    // The parent is scaled up and shouldn't need to scale during draw. The child that can scale its contents should
    // also not need to scale during draw. The other should.
    // There is some rounding error due to contentBounds being a rounded integer.
    EXPECT_NEAR(parent->drawTransform().m11(), 1, 0.01);
    EXPECT_NEAR(parent->drawTransform().m22(), 1, 0.01);
    EXPECT_NEAR(childScale->drawTransform().m11(), 1, 0.01);
    EXPECT_NEAR(childScale->drawTransform().m22(), 1, 0.01);
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, childNoScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, childNoScale->drawTransform().m22());
    EXPECT_NEAR(childNoAutoScale->drawTransform().m11(), initialParentScale * initialChildScale / fixedRasterScale, 0.01);
    EXPECT_NEAR(childNoAutoScale->drawTransform().m22(), initialParentScale * initialChildScale / fixedRasterScale, 0.01);

    // If the transform changes, we expect the contentsScale to remain unchanged.
    childScale->setTransform(identityMatrix);

    renderSurfaceLayerList.clear();
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, pageScaleFactor, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * initialParentScale, parent);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, childScale);
    EXPECT_CONTENTS_SCALE_EQ(1, childNoScale);

    // But if the deviceScaleFactor or pageScaleFactor changes, then it should be updated, but using the initial transform.
    deviceScaleFactor = 2.25;
    pageScaleFactor = 1.25;

    // FIXME: Remove this when pageScaleFactor is applied in the compositor.
    pageScaleMatrix = identityMatrix;
    pageScaleMatrix.scale(pageScaleFactor);
    parent->setSublayerTransform(pageScaleMatrix);

    renderSurfaceLayerList.clear();
    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, pageScaleFactor, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * initialParentScale, parent);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, childScale);
    EXPECT_CONTENTS_SCALE_EQ(1, childNoScale);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * fixedRasterScale, childNoAutoScale);
}

TEST(LayerTreeHostCommonTest, verifyContentsScaleForSurfaces)
{
    MockContentLayerClient delegate;
    WebTransformationMatrix identityMatrix;

    WebTransformationMatrix parentScaleMatrix;
    const double initialParentScale = 2;
    parentScaleMatrix.scale(initialParentScale);

    WebTransformationMatrix childScaleMatrix;
    const double initialChildScale = 3;
    childScaleMatrix.scale(initialChildScale);

    float fixedRasterScale = 4;

    scoped_refptr<ContentLayer> parent = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(parent.get(), parentScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(100, 100), true);

    scoped_refptr<ContentLayer> surfaceScale = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(2, 2), gfx::Size(10, 10), true);

    scoped_refptr<ContentLayer> surfaceScaleChildScale = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceScaleChildScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), true);

    scoped_refptr<NoScaleContentLayer> surfaceScaleChildNoScale = createNoScaleDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceScaleChildNoScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), true);

    scoped_refptr<NoScaleContentLayer> surfaceNoScale = createNoScaleDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceNoScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(12, 12), gfx::Size(10, 10), true);

    scoped_refptr<ContentLayer> surfaceNoScaleChildScale = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceNoScaleChildScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), true);

    scoped_refptr<NoScaleContentLayer> surfaceNoScaleChildNoScale = createNoScaleDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceNoScaleChildNoScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), true);

    scoped_refptr<ContentLayer> surfaceNoAutoScale = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceNoAutoScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(22, 22), gfx::Size(10, 10), true);
    surfaceNoAutoScale->setAutomaticallyComputeRasterScale(false);
    surfaceNoAutoScale->setRasterScale(fixedRasterScale);

    scoped_refptr<ContentLayer> surfaceNoAutoScaleChildScale = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceNoAutoScaleChildScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), true);

    scoped_refptr<NoScaleContentLayer> surfaceNoAutoScaleChildNoScale = createNoScaleDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(surfaceNoAutoScaleChildNoScale.get(), childScaleMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), true);

    // FIXME: Remove this when pageScaleFactor is applied in the compositor.
    // Page scale should not apply to the parent.
    parent->setBoundsContainPageScale(true);

    parent->addChild(surfaceScale);
    parent->addChild(surfaceNoScale);
    parent->addChild(surfaceNoAutoScale);

    surfaceScale->setForceRenderSurface(true);
    surfaceScale->addChild(surfaceScaleChildScale);
    surfaceScale->addChild(surfaceScaleChildNoScale);

    surfaceNoScale->setForceRenderSurface(true);
    surfaceNoScale->addChild(surfaceNoScaleChildScale);
    surfaceNoScale->addChild(surfaceNoScaleChildNoScale);

    surfaceNoAutoScale->setForceRenderSurface(true);
    surfaceNoAutoScale->addChild(surfaceNoAutoScaleChildScale);
    surfaceNoAutoScale->addChild(surfaceNoAutoScaleChildNoScale);

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;

    double deviceScaleFactor = 5;
    double pageScaleFactor = 7;

    // FIXME: Remove this when pageScaleFactor is applied in the compositor.
    WebTransformationMatrix pageScaleMatrix;
    pageScaleMatrix.scale(pageScaleFactor);
    parent->setSublayerTransform(pageScaleMatrix);

    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, pageScaleFactor, dummyMaxTextureSize, renderSurfaceLayerList);

    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * initialParentScale, parent);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, surfaceScale);
    EXPECT_CONTENTS_SCALE_EQ(1, surfaceNoScale);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * fixedRasterScale, surfaceNoAutoScale);

    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceScaleChildScale);
    EXPECT_CONTENTS_SCALE_EQ(1, surfaceScaleChildNoScale);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceNoScaleChildScale);
    EXPECT_CONTENTS_SCALE_EQ(1, surfaceNoScaleChildNoScale);
    EXPECT_CONTENTS_SCALE_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceNoAutoScaleChildScale);
    EXPECT_CONTENTS_SCALE_EQ(1, surfaceNoAutoScaleChildNoScale);

    // The parent is scaled up and shouldn't need to scale during draw.
    EXPECT_FLOAT_EQ(1, parent->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, parent->drawTransform().m22());

    // RenderSurfaces should always be 1:1 with their target.
    EXPECT_FLOAT_EQ(1, surfaceScale->renderSurface()->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, surfaceScale->renderSurface()->drawTransform().m22());

    // The surfaceScale can apply contents scale so the layer shouldn't need to scale during draw.
    EXPECT_FLOAT_EQ(1, surfaceScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, surfaceScale->drawTransform().m22());

    // The surfaceScaleChildScale can apply contents scale so it shouldn't need to scale during draw.
    EXPECT_FLOAT_EQ(1, surfaceScaleChildScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, surfaceScaleChildScale->drawTransform().m22());

    // The surfaceScaleChildNoScale can not apply contents scale, so it needs to be scaled during draw.
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceScaleChildNoScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceScaleChildNoScale->drawTransform().m22());

    // RenderSurfaces should always be 1:1 with their target.
    EXPECT_FLOAT_EQ(1, surfaceNoScale->renderSurface()->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, surfaceNoScale->renderSurface()->drawTransform().m22());

    // The surfaceNoScale layer can not apply contents scale, so it needs to be scaled during draw.
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, surfaceNoScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale, surfaceNoScale->drawTransform().m22());

    // The surfaceScaleChildScale can apply contents scale so it shouldn't need to scale during draw.
    EXPECT_FLOAT_EQ(1, surfaceNoScaleChildScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, surfaceNoScaleChildScale->drawTransform().m22());

    // The surfaceScaleChildNoScale can not apply contents scale, so it needs to be scaled during draw.
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceNoScaleChildNoScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceNoScaleChildNoScale->drawTransform().m22());

    // RenderSurfaces should always be 1:1 with their target.
    EXPECT_FLOAT_EQ(1, surfaceNoAutoScale->renderSurface()->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, surfaceNoAutoScale->renderSurface()->drawTransform().m22());

    // The surfaceNoAutoScale layer has a fixed contents scale, so it needs to be scaled during draw.
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale / (deviceScaleFactor * pageScaleFactor * fixedRasterScale), surfaceNoAutoScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale / (deviceScaleFactor * pageScaleFactor * fixedRasterScale), surfaceNoAutoScale->drawTransform().m22());

    // The surfaceScaleChildScale can apply contents scale so it shouldn't need to scale during draw.
    EXPECT_FLOAT_EQ(1, surfaceNoAutoScaleChildScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(1, surfaceNoAutoScaleChildScale->drawTransform().m22());

    // The surfaceScaleChildNoScale can not apply contents scale, so it needs to be scaled during draw.
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceNoAutoScaleChildNoScale->drawTransform().m11());
    EXPECT_FLOAT_EQ(deviceScaleFactor * pageScaleFactor * initialParentScale * initialChildScale * initialChildScale, surfaceNoAutoScaleChildNoScale->drawTransform().m22());
}

TEST(LayerTreeHostCommonTest, verifyRenderSurfaceTransformsInHighDPI)
{
    MockContentLayerClient delegate;
    WebTransformationMatrix identityMatrix;

    scoped_refptr<ContentLayer> parent = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(30, 30), true);

    scoped_refptr<ContentLayer> child = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(2, 2), gfx::Size(10, 10), true);

    WebTransformationMatrix replicaTransform;
    replicaTransform.scaleNonUniform(1, -1);
    scoped_refptr<ContentLayer> replica = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(replica.get(), replicaTransform, identityMatrix, gfx::PointF(0, 0), gfx::PointF(2, 2), gfx::Size(10, 10), true);

    // This layer should end up in the same surface as child, with the same draw
    // and screen space transforms.
    scoped_refptr<ContentLayer> duplicateChildNonOwner = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(duplicateChildNonOwner.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(10, 10), true);

    parent->addChild(child);
    child->addChild(duplicateChildNonOwner);
    child->setReplicaLayer(replica.get());

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;

    const double deviceScaleFactor = 1.5;
    parent->setContentsScale(deviceScaleFactor);
    child->setContentsScale(deviceScaleFactor);
    duplicateChildNonOwner->setContentsScale(deviceScaleFactor);
    replica->setContentsScale(deviceScaleFactor);

    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // We should have two render surfaces. The root's render surface and child's
    // render surface (it needs one because it has a replica layer).
    EXPECT_EQ(2u, renderSurfaceLayerList.size());

    WebTransformationMatrix expectedParentTransform;
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedParentTransform, parent->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedParentTransform, parent->drawTransform());

    WebTransformationMatrix expectedDrawTransform;
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedDrawTransform, child->drawTransform());

    WebTransformationMatrix expectedScreenSpaceTransform;
    expectedScreenSpaceTransform.translate(deviceScaleFactor * child->position().x(), deviceScaleFactor * child->position().y());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedScreenSpaceTransform, child->screenSpaceTransform());

    WebTransformationMatrix expectedDuplicateChildDrawTransform = child->drawTransform();
    EXPECT_TRANSFORMATION_MATRIX_EQ(child->drawTransform(), duplicateChildNonOwner->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(child->screenSpaceTransform(), duplicateChildNonOwner->screenSpaceTransform());
    EXPECT_RECT_EQ(child->drawableContentRect(), duplicateChildNonOwner->drawableContentRect());
    EXPECT_EQ(child->contentBounds(), duplicateChildNonOwner->contentBounds());

    WebTransformationMatrix expectedRenderSurfaceDrawTransform;
    expectedRenderSurfaceDrawTransform.translate(deviceScaleFactor * child->position().x(), deviceScaleFactor * child->position().y());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedRenderSurfaceDrawTransform, child->renderSurface()->drawTransform());

    WebTransformationMatrix expectedSurfaceDrawTransform;
    expectedSurfaceDrawTransform.translate(deviceScaleFactor * 2, deviceScaleFactor * 2);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedSurfaceDrawTransform, child->renderSurface()->drawTransform());

    WebTransformationMatrix expectedSurfaceScreenSpaceTransform;
    expectedSurfaceScreenSpaceTransform.translate(deviceScaleFactor * 2, deviceScaleFactor * 2);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedSurfaceScreenSpaceTransform, child->renderSurface()->screenSpaceTransform());

    WebTransformationMatrix expectedReplicaDrawTransform;
    expectedReplicaDrawTransform.setM22(-1);
    expectedReplicaDrawTransform.setM41(6);
    expectedReplicaDrawTransform.setM42(6);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedReplicaDrawTransform, child->renderSurface()->replicaDrawTransform());

    WebTransformationMatrix expectedReplicaScreenSpaceTransform;
    expectedReplicaScreenSpaceTransform.setM22(-1);
    expectedReplicaScreenSpaceTransform.setM41(6);
    expectedReplicaScreenSpaceTransform.setM42(6);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedReplicaScreenSpaceTransform, child->renderSurface()->replicaScreenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedReplicaScreenSpaceTransform, child->renderSurface()->replicaScreenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifyRenderSurfaceTransformsInHighDPIAccurateScaleZeroPosition)
{
    MockContentLayerClient delegate;
    WebTransformationMatrix identityMatrix;

    scoped_refptr<ContentLayer> parent = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(parent.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(33, 31), true);

    scoped_refptr<ContentLayer> child = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(child.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(13, 11), true);

    WebTransformationMatrix replicaTransform;
    replicaTransform.scaleNonUniform(1, -1);
    scoped_refptr<ContentLayer> replica = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(replica.get(), replicaTransform, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(13, 11), true);

    // This layer should end up in the same surface as child, with the same draw
    // and screen space transforms.
    scoped_refptr<ContentLayer> duplicateChildNonOwner = createDrawableContentLayer(&delegate);
    setLayerPropertiesForTesting(duplicateChildNonOwner.get(), identityMatrix, identityMatrix, gfx::PointF(0, 0), gfx::PointF(0, 0), gfx::Size(13, 11), true);

    parent->addChild(child);
    child->addChild(duplicateChildNonOwner);
    child->setReplicaLayer(replica.get());

    std::vector<scoped_refptr<Layer> > renderSurfaceLayerList;
    int dummyMaxTextureSize = 512;

    const float deviceScaleFactor = 1.7f;
    parent->setContentsScale(deviceScaleFactor);
    child->setContentsScale(deviceScaleFactor);
    duplicateChildNonOwner->setContentsScale(deviceScaleFactor);
    replica->setContentsScale(deviceScaleFactor);

    LayerTreeHostCommon::calculateDrawTransforms(parent.get(), parent->bounds(), deviceScaleFactor, 1, dummyMaxTextureSize, renderSurfaceLayerList);

    // We should have two render surfaces. The root's render surface and child's
    // render surface (it needs one because it has a replica layer).
    EXPECT_EQ(2u, renderSurfaceLayerList.size());

    WebTransformationMatrix identityTransform;

    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, parent->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, parent->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, child->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, child->screenSpaceTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, duplicateChildNonOwner->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, duplicateChildNonOwner->screenSpaceTransform());
    EXPECT_RECT_EQ(child->drawableContentRect(), duplicateChildNonOwner->drawableContentRect());
    EXPECT_EQ(child->contentBounds(), duplicateChildNonOwner->contentBounds());

    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, child->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, child->renderSurface()->drawTransform());
    EXPECT_TRANSFORMATION_MATRIX_EQ(identityTransform, child->renderSurface()->screenSpaceTransform());

    WebTransformationMatrix expectedReplicaDrawTransform;
    expectedReplicaDrawTransform.setM22(-1);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedReplicaDrawTransform, child->renderSurface()->replicaDrawTransform());

    WebTransformationMatrix expectedReplicaScreenSpaceTransform;
    expectedReplicaScreenSpaceTransform.setM22(-1);
    EXPECT_TRANSFORMATION_MATRIX_EQ(expectedReplicaScreenSpaceTransform, child->renderSurface()->replicaScreenSpaceTransform());
}

TEST(LayerTreeHostCommonTest, verifySubtreeSearch)
{
    scoped_refptr<Layer> root = Layer::create();
    scoped_refptr<Layer> child = Layer::create();
    scoped_refptr<Layer> grandChild = Layer::create();
    scoped_refptr<Layer> maskLayer = Layer::create();
    scoped_refptr<Layer> replicaLayer = Layer::create();

    grandChild->setReplicaLayer(replicaLayer.get());
    child->addChild(grandChild.get());
    child->setMaskLayer(maskLayer.get());
    root->addChild(child.get());

    int nonexistentId = -1;
    EXPECT_EQ(root, LayerTreeHostCommon::findLayerInSubtree(root.get(), root->id()));
    EXPECT_EQ(child, LayerTreeHostCommon::findLayerInSubtree(root.get(), child->id()));
    EXPECT_EQ(grandChild, LayerTreeHostCommon::findLayerInSubtree(root.get(), grandChild->id()));
    EXPECT_EQ(maskLayer, LayerTreeHostCommon::findLayerInSubtree(root.get(), maskLayer->id()));
    EXPECT_EQ(replicaLayer, LayerTreeHostCommon::findLayerInSubtree(root.get(), replicaLayer->id()));
    EXPECT_EQ(0, LayerTreeHostCommon::findLayerInSubtree(root.get(), nonexistentId));
}

} // namespace
