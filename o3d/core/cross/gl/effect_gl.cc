/*
 * Copyright 2009, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


// This file contains the definition of EffectGL, the OpenGL implementation
// of the abstract O3D class Effect.


// Disable pointer casting warning for openGL calls that require a void* to
// be cast to a GLuint
#if defined(OS_WIN)
#pragma warning(disable : 4312)
#pragma warning(disable : 4311)
#endif

#include <sstream>
#include "base/cross/std_functional.h"
#include "core/cross/semantic_manager.h"
#include "core/cross/error.h"
#include "core/cross/standard_param.h"
#include "core/cross/gl/effect_gl.h"
#include "core/cross/gl/renderer_gl.h"
#include "core/cross/gl/primitive_gl.h"
#include "core/cross/gl/draw_element_gl.h"
#include "core/cross/gl/texture_gl.h"
#include "core/cross/gl/utils_gl.h"
#include "core/cross/gl/utils_gl-inl.h"

#if defined(OS_WIN)
#include "core/cross/core_metrics.h"
#endif

namespace o3d {

// Number of repeating events to log before giving up, e.g. setup frame,
// draw polygons, etc.
const int kNumLoggedEvents = 5;

// Convert a CGparameter data type into a Param type. Note that
// Param::BOOLEAN is never generated by the Cg compiler.
static const ObjectBase::Class* CgTypeToParamType(CGtype cg_type) {
  switch (cg_type) {
    case CG_FLOAT       :
    case CG_FLOAT1      : return ParamFloat::GetApparentClass();
    case CG_FLOAT2      : return ParamFloat2::GetApparentClass();
    case CG_FLOAT3      : return ParamFloat3::GetApparentClass();
    case CG_FLOAT4      : return ParamFloat4::GetApparentClass();
    case CG_INT         : return ParamInteger::GetApparentClass();
    case CG_INT1        : return ParamInteger::GetApparentClass();
    case CG_FLOAT4x4    : return ParamMatrix4::GetApparentClass();
    case CG_BOOL        :
    case CG_BOOL1       : return ParamBoolean::GetApparentClass();
    case CG_SAMPLER     :
    case CG_SAMPLER1D   :
    case CG_SAMPLER2D   :
    case CG_SAMPLER3D   :
    case CG_SAMPLERCUBE : return ParamSampler::GetApparentClass();
    default : {
      DLOG(ERROR) << "Cannot convert CGtype "
                  << cgGetTypeString(cg_type)
                  << " to a Param type.";
      return NULL;
    }
  }
}

// -----------------------------------------------------------------------------

EffectGL::EffectGL(ServiceLocator* service_locator, CGcontext cg_context)
    : Effect(service_locator),
      semantic_manager_(service_locator->GetService<SemanticManager>()),
      renderer_(static_cast<RendererGL*>(
          service_locator->GetService<Renderer>())),
      cg_context_(cg_context),
      cg_vertex_(NULL),
      cg_fragment_(NULL) {
  DLOG(INFO) << "EffectGL Construct";
}

// Destructor releases vertex and fragment shaders and their correspoding
// constants tables.
EffectGL::~EffectGL() {
  DLOG(INFO) << "EffectGL Destruct \"" << name() << "\"";
  if (cg_vertex_) {
    cgDestroyProgram(cg_vertex_);
  }
  if (cg_fragment_) {
    cgDestroyProgram(cg_fragment_);
  }
}

// Rewrites vertex program assembly code to match GL semantics for clipping.
// This parses the source, breaking it down into pieces:
// - declaration ("!!ARBvp1.0")
// - comments (that contain the parameter information)
// - instructions
// - "END" token.
// Then it rewrites the instructions so that 'result.position' doesn't get
// written directly, instead it is written to a temporary variable. Then a
// transformation is done on that variable before outputing to
// 'result.position':
// - offset x and y by half a pixel (times w).
// - remap z from [0..w] to [-w..w].
// - invert y, if render targets are active.
//
// Note that for the 1/2 pixel offset, we need a parameter that depends on the
// current viewport. This is done through 'program.env[0]' which is shared
// across all programs (so we only have to update it once when we change the
// viewport), because Cg won't use them currently (it uses 'program.local'
// instead).
static bool RewriteVertexProgramSource(String *source) {
  String::size_type pos = source->find('\n');
  if (pos == String::npos) {
    DLOG(ERROR) << "could not find program declaration";
    return false;
  }
  String decl(*source, 0, pos + 1);
  String::size_type start_comments = pos + 1;
  // skip the comments that contain the parameters etc.
  for (; pos < source->size(); pos = source->find('\n', pos)) {
    ++pos;
    if (pos >= source->size())
      break;
    if ((*source)[pos] != '#')
      break;
  }
  if (pos >= source->size()) {
    // we only found comments.
    return false;
  }
  String comments(*source, start_comments, pos - start_comments);

  String::size_type end_token = source->find("\nEND", pos + 1);
  if (end_token == String::npos) {
    DLOG(ERROR) << "Compiled shader doesn't have an END token";
    return false;
  }
  String instructions(*source, pos, end_token + 1 - pos);

  // Replace accesses to 'result.position' by accesses to our temp variable
  // '$O3D_HPOS'.
  // '$' is a valid symbol for identifiers, but Cg doesn't seem to be using
  // it, so we can use it to ensure we don't have name conflicts.
  static const char kOutPositionRegister[] = "result.position";
  for (String::size_type i = instructions.find(kOutPositionRegister);
       i < String::npos; i = instructions.find(kOutPositionRegister, i)) {
    instructions.replace(i, strlen(kOutPositionRegister), "$O3D_HPOS");
  }

  *source = decl +
      comments +
      // .x = 1/viewport.width; .y = -1/viewport.height; .z = 2.0; w = +/-1.0;
      "PARAM $O3D_HELPER = program.env[0];\n"
      "TEMP $O3D_HPOS;\n" +
      instructions +
      // hpos.x <- hpos.x + hpos.w / viewport.width;
      // hpos.y <- hpos.y - hpos.w / viewport.height;
      "MAD $O3D_HPOS.xy, $O3D_HELPER.xyyy, $O3D_HPOS.w, $O3D_HPOS.xyyy;\n"
      // Invert y, based on the w component of the helper arg.
      "MUL $O3D_HPOS.y, $O3D_HELPER.w, $O3D_HPOS.y;\n"
      // hpos.z <- hpos.z * 2 - hpos.w
      "MAD $O3D_HPOS.z, $O3D_HPOS.z, $O3D_HELPER.z, -$O3D_HPOS.w;\n"
      "MOV result.position, $O3D_HPOS;\n"
      "END\n";
  return true;
}

// Initializes the Effect object using the shaders found in an FX formatted
// string.
bool EffectGL::LoadFromFXString(const String& effect) {
  DLOG(INFO) << "EffectGL LoadFromFXString";
  renderer_->MakeCurrentLazy();

  set_source("");

  String vertex_shader_entry_point;
  String fragment_shader_entry_point;
  MatrixLoadOrder matrix_load_order;
  // TODO(gman): Check for failure once shader parser is in.
  if (!ValidateFX(effect,
                  &vertex_shader_entry_point,
                  &fragment_shader_entry_point,
                  &matrix_load_order)) {
    return false;
  }

  set_matrix_load_order(matrix_load_order);

  // Compile the original vertex program once, to get the ARBVP1 assembly code.
  CGprogram original_vp = cgCreateProgram(cg_context_, CG_SOURCE,
                                          effect.c_str(), CG_PROFILE_ARBVP1,
                                          vertex_shader_entry_point.c_str(),
                                          NULL);
  const char* listing = cgGetLastListing(cg_context_);
  if (original_vp == NULL) {
    O3D_ERROR(service_locator()) << "Effect Compile Error: "
                                 << cgGetErrorString(cgGetError()) << " : "
                                 << listing;
    return false;
  }

  if (listing && listing[0] != 0) {
    DLOG(WARNING) << "Effect Compile Warnings: " << listing;
  }

  String vp_assembly = cgGetProgramString(original_vp, CG_COMPILED_PROGRAM);
  cgDestroyProgram(original_vp);
  if (!RewriteVertexProgramSource(&vp_assembly)) {
    return false;
  }
  cg_vertex_ = cgCreateProgram(cg_context_, CG_OBJECT, vp_assembly.c_str(),
                               CG_PROFILE_ARBVP1,
                               vertex_shader_entry_point.c_str(), NULL);
  listing = cgGetLastListing(cg_context_);
  if (cg_vertex_ == NULL) {
    O3D_ERROR(service_locator()) << "Effect post-rewrite Compile Error: "
                                 << cgGetErrorString(cgGetError()) << " : "
                                 << listing;
    return false;
  }

#ifdef OS_WIN
  // Get metrics for length of the vertex shader
  const char* shader_data = cgGetProgramString(cg_vertex_, CG_COMPILED_PROGRAM);
  metric_vertex_shader_instruction_count.AddSample(strlen(shader_data));
#endif

  if (listing && listing[0] != 0) {
    DLOG(WARNING) << "Effect post-rewrite compile warnings: " << listing;
  }

  // If the program rewrite introduced some syntax or semantic errors, we won't
  // know it until we load the program (through a GL error).
  // So flush all GL errors first...
  FlushGlErrors();

  // ... Then load the program ...
  cgGLLoadProgram(cg_vertex_);

  // ... And check for GL errors.
  if (glGetError() != GL_NO_ERROR) {
    O3D_ERROR(service_locator())
        << "Effect post-rewrite GL Error: "
        << glGetString(GL_PROGRAM_ERROR_STRING_ARB)
        << "\nSource: \n"
        << vp_assembly;
    return false;
  }

  cg_fragment_ = cgCreateProgram(cg_context_, CG_SOURCE, effect.c_str(),
                                 CG_PROFILE_ARBFP1,
                                 fragment_shader_entry_point.c_str(), NULL);
  listing = cgGetLastListing(cg_context_);
  if (cg_fragment_ == NULL) {
    O3D_ERROR(service_locator()) << "Effect Compile Error: "
                                 << cgGetErrorString(cgGetError()) << " : "
                                 << listing;
    return false;
  }

#ifdef OS_WIN
  // Get metrics for length of the fragment shader
  shader_data = cgGetProgramString(cg_fragment_, CG_COMPILED_PROGRAM);
  metric_pixel_shader_instruction_count.AddSample(strlen(shader_data));
#endif

  if (listing && listing[0] != 0) {
    DLOG(WARNING) << "Effect Compile Warnings: " << listing;
  }

  cgGLLoadProgram(cg_fragment_);

  // Also check for GL errors, in case Cg managed to compile, but generated a
  // bad program.
  if (glGetError() != GL_NO_ERROR) {
    O3D_ERROR(service_locator())
        << "Effect GL Error: "
        << glGetString(GL_PROGRAM_ERROR_STRING_ARB);
    return false;
  }

  // TODO(o3d): remove this (OLD path for textures).
  FillSamplerToTextureMap(effect);

  CHECK_GL_ERROR();

  set_source(effect);
  return true;
}

// Fills the sampler->texture map. This needs to compile the source as an
// effect because the state assignments get lost when compiled as a
// shader.
// Note that we compile the raw effect, which shouldn't have any
// technique/pass, so we don't actually create programs, just parse the
// uniforms and state assignments.
void EffectGL::FillSamplerToTextureMap(const String &effect) {
  CGeffect cg_effect = cgCreateEffect(cg_context_, effect.c_str(), NULL);
  if (!cg_effect) {
    DLOG(ERROR) << "Could not compile the effect to find "
                << "Sampler->Texture associations";
    return;
  }
  for (CGparameter param = cgGetFirstEffectParameter(cg_effect);
       param; param = cgGetNextLeafParameter(param)) {
    CGtype cg_type = cgGetParameterType(param);
    switch (cg_type) {
      case CG_SAMPLER:
      case CG_SAMPLER1D:
      case CG_SAMPLER2D:
      case CG_SAMPLER3D:
      case CG_SAMPLERCUBE:
        break;
      default:
        continue;
    }
    CGstateassignment state_assignment =
        cgGetNamedSamplerStateAssignment(param, "Texture");
    if (!state_assignment)
      continue;
    CGparameter texture_param =
        cgGetTextureStateAssignmentValue(state_assignment);
    if (!texture_param)
      continue;
    DCHECK((cgGetParameterType(texture_param)  == CG_TEXTURE));
    sampler_to_texture_map_[cgGetParameterName(param)] =
        cgGetParameterName(texture_param);
  }
  cgDestroyEffect(cg_effect);
}

// TODO(o3d): remove this (OLD path for textures).
String EffectGL::GetTextureNameFromSamplerParamName(
    const String &sampler_name) {
  std::map<String, String>::iterator it =
      sampler_to_texture_map_.find(sampler_name);
  if (it != sampler_to_texture_map_.end()) {
    return it->second;
  } else {
    return "";
  }
}

// Given a CG_SAMPLER parameter, find the corresponding CG_TEXTURE
// parameter. From this CG_TEXTURE, find a matching Param by name.
ParamTexture* EffectGL::GetTextureParamFromCgSampler(
    CGparameter cg_sampler,
    const std::vector<ParamObject*> &param_objects) {
  DLOG(INFO) << "EffectGL GetTextureParamFromCgSampler";
  DLOG_ASSERT(cgGetParameterType(cg_sampler) != CG_SAMPLER);
  String sampler_name = cgGetParameterName(cg_sampler);
  String param_name = GetTextureNameFromSamplerParamName(sampler_name);
  if (param_name.size() == 0) {
    // Sampler has no texture associated with it.
    return NULL;
  }
  // Find a matching Param with the same name as the CG_TEXTURE.
  for (unsigned int i = 0; i < param_objects.size(); ++i) {
    Param* param = param_objects[i]->GetUntypedParam(param_name);
    if (param && param->IsA(ParamTexture::GetApparentClass())) {
      // Success.
      DLOG(INFO) << "EffectGL Matched CG_SAMPLER \""
                 << sampler_name
                 << "\" To Param \""
                 << param_name << "\"";
      return down_cast<ParamTexture*>(param);
    }
  }
  DLOG(INFO) << "No matching Param for CG_TEXTURE \""
             << param_name
             << "\" used by CG_SAMPLER \""
             << sampler_name << "\"";
  return NULL;
}

void EffectGL::GetShaderParamInfo(
    CGprogram program,
    CGenum name_space,
    std::map<String, EffectParameterInfo>* info_map) {
  DCHECK(info_map);

  // Loop over all parameters, visiting only CGparameters that have
  // had storage allocated to them.
  CGparameter cg_param = cgGetFirstParameter(program, name_space);
  for (; cg_param != NULL; cg_param = cgGetNextParameter(cg_param)) {
    CGenum variability = cgGetParameterVariability(cg_param);
    if (variability != CG_UNIFORM)
      continue;
    CGenum direction = cgGetParameterDirection(cg_param);
    if (direction != CG_IN)
      continue;
    String name = cgGetParameterName(cg_param);
    CGtype cg_type = cgGetParameterType(cg_param);
    // Texture parameters need special handling as the c3cImport system
    // records a handle to the CG_TEXTURE param, not the CG_SAMPLER
    // param. D3D sets textures by binding a bitmap to the Texture
    // param, Cg binds bitmaps to the Sampler parameter. We solve this
    // by keeping an internal collection of Texture-Sampler mappings
    // that is built here, so we can later to do the reverse lookup.
    //
    // TODO(o3d): This will not solve the one-to-many problem of one
    // Texture being used by many Sampler params, but it's enough to get
    // us up and running.
    //
    // TODO(o3d): Once we start using samplers exclusively, this special
    // treatment of textures should go away. For the time being though, we do
    // end up creating a texture param on the param_object.
    if (cg_type == CG_SAMPLER1D ||
        cg_type == CG_SAMPLER2D ||
        cg_type == CG_SAMPLER3D ||
        cg_type == CG_SAMPLERCUBE) {
      // rename the parameter to have the name of the texture.
      String texture_param_name = GetTextureNameFromSamplerParamName(name);
      if (texture_param_name.size() != 0) {
        (*info_map)[texture_param_name] = EffectParameterInfo(
            texture_param_name,
            ParamTexture::GetApparentClass(),
            0,
            "",
            false);
      }
    } else if (cg_type == CG_TEXTURE) {
      continue;
    }
    int num_elements;
    if (cg_type == CG_ARRAY) {
      num_elements = cgGetArraySize(cg_param, 0);
      // Substitute the first element's type for our type.
      cg_type = cgGetParameterType(cgGetArrayParameter(cg_param, 0));
    } else {
      num_elements = 0;
    }
    const ObjectBase::Class *param_class = CgTypeToParamType(cg_type);
    if (!param_class)
      continue;
    const char* cg_semantic = cgGetParameterSemantic(cg_param);
    const ObjectBase::Class *sem_class = NULL;
    if (cg_semantic != NULL && cg_semantic[0] != '\0') {
      // NOTE: this semantic is not the regularised profile semantic output
      // from the CGC compiler but the actual user supplied semantic from
      // the shader source code, so this match is valid.
      sem_class = semantic_manager_->LookupSemantic(cg_semantic);
    }
    (*info_map)[name] = EffectParameterInfo(
        name,
        param_class,
        num_elements,
        (cg_semantic != NULL) ? cg_semantic : "",
        sem_class);
  }
}

void EffectGL::GetParameterInfo(EffectParameterInfoArray* info_array) {
  DCHECK(info_array);
  std::map<String, EffectParameterInfo> info_map;
  renderer_->MakeCurrentLazy();
  if (cg_vertex_) {
    GetShaderParamInfo(cg_vertex_, CG_PROGRAM, &info_map);
    GetShaderParamInfo(cg_vertex_, CG_GLOBAL, &info_map);
  }
  if (cg_fragment_) {
    // create Param objects based on the parameters found in the fragment
    // program.
    GetShaderParamInfo(cg_fragment_, CG_PROGRAM, &info_map);
    GetShaderParamInfo(cg_fragment_, CG_GLOBAL, &info_map);
  }
  info_array->clear();
  info_array->reserve(info_map.size());
  std::transform(
      info_map.begin(),
      info_map.end(),
      std::back_inserter(*info_array),
      base::select2nd<std::map<String, EffectParameterInfo>::value_type>());
}

void EffectGL::GetVaryingVertexShaderParamInfo(
    CGprogram program,
    CGenum name_space,
    std::vector<EffectStreamInfo>* info_array) {
  CGparameter cg_param = cgGetFirstLeafParameter(cg_vertex_, name_space);
  for (; cg_param != NULL; cg_param = cgGetNextLeafParameter(cg_param)) {
    CGenum variability = cgGetParameterVariability(cg_param);
    if (variability != CG_VARYING)
      continue;
    CGenum direction = cgGetParameterDirection(cg_param);
    if (direction != CG_IN)
      continue;

    const char* cg_semantic = cgGetParameterSemantic(cg_param);
    if (cg_semantic == NULL)
      continue;

    int attr = SemanticNameToGLVertexAttribute(cg_semantic);
    if (attr < 0)
      continue;

    int semantic_index = 0;
    Stream::Semantic semantic = GLVertexAttributeToStream(attr,
                                                          &semantic_index);
    if (semantic == Stream::UNKNOWN_SEMANTIC)
      continue;

    info_array->push_back(EffectStreamInfo(semantic,
                                           semantic_index));
  }
}

void EffectGL::GetStreamInfo(
    EffectStreamInfoArray* info_array) {
  DCHECK(info_array);
  renderer_->MakeCurrentLazy();
  info_array->clear();
  GetVaryingVertexShaderParamInfo(cg_vertex_, CG_PROGRAM, info_array);
  GetVaryingVertexShaderParamInfo(cg_vertex_, CG_GLOBAL, info_array);
}


// private functions -----------------------------------------------------------

// Loop over all the CG_SAMPLER objects and attach the GLuint handle for the
// GL texture object that we discovered earlier. Then execute the
// CGstateassignments in the sampler_state to set up the texture unit.
// TODO(o3d): remove this (OLD path for textures).
void EffectGL::SetTexturesFromEffect(ParamCacheGL* param_cache_gl) {
  DLOG_FIRST_N(INFO, kNumLoggedEvents) << "EffectGL EnableTexturesFromEffect";
  ParamCacheGL::SamplerParameterMap& map = param_cache_gl->sampler_map();
  ParamCacheGL::SamplerParameterMap::iterator i;
  for (i = map.begin(); i != map.end(); ++i) {
    CGparameter cg_param = i->first;
    ParamTexture *param = i->second;
    if (param != NULL) {
      Texture *t = param->value();
      if (t) {
        GLuint handle = static_cast<GLuint>(reinterpret_cast<intptr_t>(
            t->GetTextureHandle()));
        cgGLSetTextureParameter(cg_param, handle);
        cgGLEnableTextureParameter(cg_param);
      }
    }
  }
  CHECK_GL_ERROR();
}

// Loop through all the uniform CGparameters on the effect and set their
// values from their corresponding Params on the various ParamObject (as stored
// in the ParamCacheGL).
void EffectGL::UpdateShaderUniformsFromEffect(ParamCacheGL* param_cache_gl) {
  DLOG_FIRST_N(INFO, kNumLoggedEvents)
      << "EffectGL UpdateShaderUniformsFromEffect";
  ParamCacheGL::UniformParameterMap& map = param_cache_gl->uniform_map();
  ParamCacheGL::UniformParameterMap::iterator i;
  for (i = map.begin(); i != map.end(); ++i) {
    CGparameter cg_param = i->first;
    i->second->SetEffectParam(renderer_, cg_param);
  }
  CHECK_GL_ERROR();
}

// Loop through all the uniform CGparameters on the effect and reset their
// values.  For now, this unbinds textures contained in sampler parameters.
void EffectGL::ResetShaderUniforms(ParamCacheGL* param_cache_gl) {
  DLOG_FIRST_N(INFO, kNumLoggedEvents) << "EffectGL ResetShaderUniforms";
  ParamCacheGL::UniformParameterMap& map = param_cache_gl->uniform_map();
  ParamCacheGL::UniformParameterMap::iterator i;
  for (i = map.begin(); i != map.end(); ++i) {
    CGparameter cg_param = i->first;
    i->second->ResetEffectParam(renderer_, cg_param);
  }
  CHECK_GL_ERROR();
}

// Updates the values of the vertex and fragment shader parameters using the
// current values in the param/cgparam caches.
void EffectGL::PrepareForDraw(ParamCacheGL* param_cache_gl) {
  DLOG_FIRST_N(INFO, kNumLoggedEvents) << "EffectGL PrepareForDraw \""
                                       << name()
                                       << "\"";
  DCHECK(renderer_->IsCurrent());
  if (cg_vertex_ && cg_fragment_) {
    // Initialise the render states for this pass, this includes the shaders.
    cgGLBindProgram(cg_vertex_);
    cgGLBindProgram(cg_fragment_);
    UpdateShaderUniformsFromEffect(param_cache_gl);

    // TODO(o3d): remove this (OLD path for textures).
    SetTexturesFromEffect(param_cache_gl);
  } else {
    DLOG_FIRST_N(ERROR, kNumLoggedEvents)
        << "No valid CGeffect found "
        << "in Effect \"" << name() << "\"";
  }
  CHECK_GL_ERROR();
}

// Resets the render states back to their default value.
void EffectGL::PostDraw(ParamCacheGL* param_cache_gl) {
  DLOG_FIRST_N(INFO, kNumLoggedEvents)
      << "EffectGL PostDraw \"" << name() << "\"";
  DCHECK(renderer_->IsCurrent());
  ResetShaderUniforms(param_cache_gl);
  CHECK_GL_ERROR();
}

}  // namespace o3d
