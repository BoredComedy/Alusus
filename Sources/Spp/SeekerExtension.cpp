/**
 * @file Spp/SeekerExtension.cpp
 * Contains the implementation of class Spp::SeekerExtension.
 *
 * @copyright Copyright (C) 2019 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#include "spp.h"

namespace Spp
{

using namespace std::placeholders;

//==============================================================================
// Initialization Functions

SeekerExtension::Overrides* SeekerExtension::extend(Core::Data::Seeker *seeker, SharedPtr<Ast::Helper> const &astHelper)
{
  auto extension = std::make_shared<SeekerExtension>(seeker);
  seeker->addDynamicInterface(extension);

  auto overrides = new Overrides();
  extension->astHelper = astHelper;
  overrides->foreachRef = seeker->foreach.set(&SeekerExtension::_foreach).get();
  overrides->foreachByIdentifier_levelRef =
    seeker->foreachByIdentifier_level.set(&SeekerExtension::_foreachByIdentifier_level).get();
  overrides->foreachByIdentifier_blockRef =
    extension->foreachByIdentifier_block.set(&SeekerExtension::_foreachByIdentifier_block).get();
  overrides->foreachByIdentifier_functionRef =
    extension->foreachByIdentifier_function.set(&SeekerExtension::_foreachByIdentifier_function).get();
  overrides->foreachByParamPassRef =
    extension->foreachByParamPass.set(&SeekerExtension::_foreachByParamPass).get();
  overrides->foreachByParamPass_routingRef =
    extension->foreachByParamPass_routing.set(&SeekerExtension::_foreachByParamPass_routing).get();
  overrides->foreachByParamPass_templateRef =
    extension->foreachByParamPass_template.set(&SeekerExtension::_foreachByParamPass_template).get();

  return overrides;
}


void SeekerExtension::unextend(Core::Data::Seeker *seeker, Overrides *overrides)
{
  auto extension = ti_cast<SeekerExtension>(seeker);
  seeker->foreach.reset(overrides->foreachRef);
  seeker->foreachByIdentifier_level.reset(overrides->foreachByIdentifier_levelRef);
  extension->astHelper = SharedPtr<Ast::Helper>::null;
  extension->foreachByIdentifier_block.reset(overrides->foreachByIdentifier_blockRef);
  extension->foreachByIdentifier_function.reset(overrides->foreachByIdentifier_functionRef);
  extension->foreachByParamPass.reset(overrides->foreachByParamPassRef);
  extension->foreachByParamPass_routing.reset(overrides->foreachByParamPass_routingRef);
  extension->foreachByParamPass_template.reset(overrides->foreachByParamPass_templateRef);

  seeker->removeDynamicInterface<SeekerExtension>();
  delete overrides;
}


//==============================================================================
// Seek Functions

Core::Data::Seeker::Verb SeekerExtension::_foreach(
  TiFunctionBase *base, TiObject *self, TiObject const *ref, TiObject *target,
  Core::Data::Seeker::ForeachCallback const &cb, Word flags
) {
  if (ref->isA<Data::Ast::ParamPass>()) {
    PREPARE_SELF(seekerExtension, SeekerExtension);
    return seekerExtension->foreachByParamPass(static_cast<Data::Ast::ParamPass const*>(ref), target, cb, flags);
  } else {
    PREPARE_SELF(seeker, Core::Data::Seeker);
    return seeker->foreach.useCallee(base)(ref, target, cb, flags);
  }
}


Core::Data::Seeker::Verb SeekerExtension::_foreachByIdentifier_level(
  TiFunctionBase *base, TiObject *self, Data::Ast::Identifier const *identifier, TiObject *data,
  Core::Data::Seeker::ForeachCallback const &cb, Word flags
) {
  if (data->isDerivedFrom<Ast::Block>()) {
    PREPARE_SELF(seekerExtension, SeekerExtension);
    return seekerExtension->foreachByIdentifier_block(identifier, static_cast<Ast::Block*>(data), cb, flags);
  } else if (data->isDerivedFrom<Ast::Function>()) {
    PREPARE_SELF(seekerExtension, SeekerExtension);
    return seekerExtension->foreachByIdentifier_function(identifier, static_cast<Ast::Function*>(data), cb, flags);
  } else {
    PREPARE_SELF(seeker, Core::Data::Seeker);
    return seeker->foreachByIdentifier_level.useCallee(base)(identifier, data, cb, flags);
  }
}


Core::Data::Seeker::Verb SeekerExtension::_foreachByIdentifier_block(
  TiObject *self, Data::Ast::Identifier const *identifier, Ast::Block *block,
  Core::Data::Seeker::ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Core::Data::Seeker);
  Core::Data::Seeker::Verb verb = seeker->foreachByIdentifier_scope(identifier, block, cb, flags);
  if (!Core::Data::Seeker::isMove(verb)) return verb;

  if (!(flags & Core::Data::Seeker::Flags::SKIP_USES)) {
    PREPARE_SELF(seeker, Core::Data::Seeker);
    for (Int i = 0; i < block->getUseStatementCount(); ++i) {
      auto useRef = block->getUseStatement(i);
      // If the thing we are looking for is actually this useRef, then we need to skip, otherwise we end up in
      // an infinite loop.
      if (useRef->getTarget() == identifier) continue;
      verb = seeker->foreach(useRef->getTarget().get(), block,
        [=](TiObject *o, Core::Notices::Notice*)->Core::Data::Seeker::Verb {
          if (o != 0) return seeker->foreachByIdentifier_level(identifier, o, cb, flags);
          else return Core::Data::Seeker::Verb::MOVE;
        },
        flags
      );
      if (!Core::Data::Seeker::isMove(verb)) return verb;
    }
  }
  return verb;
}


Core::Data::Seeker::Verb SeekerExtension::_foreachByIdentifier_function(
  TiObject *self, Data::Ast::Identifier const *identifier, Ast::Function *function,
  Core::Data::Seeker::ForeachCallback const &cb, Word flags
) {
  auto argTypes = function->getType()->getArgTypes().get();
  if (argTypes == 0) return Core::Data::Seeker::Verb::MOVE;
  auto index = argTypes->findIndex(identifier->getValue().get());
  if (index >= 0) return cb(argTypes->getElement(index), 0);
  return Core::Data::Seeker::Verb::MOVE;
}


Core::Data::Seeker::Verb SeekerExtension::_foreachByParamPass(
  TiObject *self, Data::Ast::ParamPass const *paramPass, TiObject *data,
  Core::Data::Seeker::ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seeker, Core::Data::Seeker);
  PREPARE_SELF(seekerExtension, SeekerExtension);
  auto operand = paramPass->getOperand().get();
  return seeker->foreach(operand, data,
    [=](TiObject *newData, Core::Notices::Notice*)->Core::Data::Seeker::Verb
    {
      return seekerExtension->foreachByParamPass_routing(paramPass, newData, cb, flags);
    },
    flags
  );
}


Core::Data::Seeker::Verb SeekerExtension::_foreachByParamPass_routing(
  TiObject *self, Data::Ast::ParamPass const *paramPass, TiObject *data,
  Core::Data::Seeker::ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seekerExtension, SeekerExtension);
  if (paramPass->getType() == Core::Data::Ast::BracketType::SQUARE) {
    auto param = paramPass->getParam().get();
    if (data->isDerivedFrom<Ast::Template>()) {
      return seekerExtension->foreachByParamPass_template(param, static_cast<Ast::Template*>(data), cb, flags);
    } else {
      throw EXCEPTION(InvalidArgumentException, S("data"), S("Unrecognized target data type."));
    }
  } else {
    throw EXCEPTION(InvalidArgumentException, S("paramPass"), S("Invalid bracket type."), paramPass->getType());
  }
}


Core::Data::Seeker::Verb SeekerExtension::_foreachByParamPass_template(
  TiObject *self, TiObject *param, Ast::Template *tmplt, Core::Data::Seeker::ForeachCallback const &cb, Word flags
) {
  PREPARE_SELF(seekerExtension, SeekerExtension);
  TioSharedPtr result;
  if (tmplt->matchInstance(param, seekerExtension->astHelper, result)) {
    return cb(result.get(), 0);
  } else {
    auto notice = result.ti_cast_get<Core::Notices::Notice>();
    if (notice != 0) return cb(0, notice);
    else return Core::Data::Seeker::Verb::MOVE;
  }
}

} // namespace
