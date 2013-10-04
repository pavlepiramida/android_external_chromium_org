// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/debug/ordered_texture_map.h"

#include "base/logging.h"
#include "cc/debug/test_texture.h"

namespace cc {

OrderedTextureMap::OrderedTextureMap() {}

OrderedTextureMap::~OrderedTextureMap() {}

void OrderedTextureMap::Append(WebKit::WebGLId id,
                               scoped_refptr<TestTexture> texture) {
  DCHECK(texture.get());
  DCHECK(!ContainsId(id));

  textures_[id] = texture;
  ordered_textures_.push_back(id);
}

void OrderedTextureMap::Replace(WebKit::WebGLId id,
                                scoped_refptr<TestTexture> texture) {
  DCHECK(texture.get());
  DCHECK(ContainsId(id));

  textures_[id] = texture;
}

void OrderedTextureMap::Remove(WebKit::WebGLId id) {
  TextureMap::iterator map_it = textures_.find(id);
  DCHECK(map_it != textures_.end());
  textures_.erase(map_it);

  TextureList::iterator list_it =
      std::find(ordered_textures_.begin(), ordered_textures_.end(), id);
  DCHECK(list_it != ordered_textures_.end());
  ordered_textures_.erase(list_it);
}

size_t OrderedTextureMap::Size() { return ordered_textures_.size(); }

bool OrderedTextureMap::ContainsId(WebKit::WebGLId id) {
  return textures_.find(id) != textures_.end();
}

scoped_refptr<TestTexture> OrderedTextureMap::TextureForId(WebKit::WebGLId id) {
  DCHECK(ContainsId(id));
  scoped_refptr<TestTexture> texture = textures_[id];
  DCHECK(texture.get());
  return texture;
}

WebKit::WebGLId OrderedTextureMap::IdAt(size_t index) {
  DCHECK(index < Size());
  return ordered_textures_[index];
}

}  // namespace cc
