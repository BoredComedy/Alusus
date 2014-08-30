/**
 * @file Core/Data/RawIndirectReference.h
 * Contains the header of class Core::Data::RawIndirectReference.
 *
 * @copyright Copyright (C) 2014 Sarmad Khalid Abdullah
 *
 * @license This file is released under Alusus Public License, Version 1.0.
 * For details on usage and copying conditions read the full license in the
 * accompanying license file or at <http://alusus.net/alusus_license_1_0>.
 */
//==============================================================================

#ifndef DATA_RAWINDIRECTREFERENCE_H
#define DATA_RAWINDIRECTREFERENCE_H

namespace Core { namespace Data
{

// TODO: DOC

class RawIndirectReference : public Reference
{
  //============================================================================
  // Type Info

  TYPE_INFO(RawIndirectReference, Reference, "Core.Data", "Core", "alusus.net");


  //============================================================================
  // Member Variables

  private: Char const *qualifier;
  private: Int size;


  //============================================================================
  // Constructor

  public: RawIndirectReference() : qualifier(0), size(0)
  {
  }

  public: RawIndirectReference(Char const *q, Int c) : qualifier(q), size(c)
  {
  }

  public: virtual ~RawIndirectReference()
  {
  }


  //============================================================================
  // Member Functions

  public: void setQualifier(Char const *q, Int s)
  {
    this->qualifier = q;
    this->size = s;
  }

  public: Char const* getQualifier() const
  {
    return this->qualifier;
  }

  public: Int getSize() const
  {
    return this->size;
  }

  public: virtual Bool compare(Reference const *r) const;

  public: virtual void setUsageCriteria(ReferenceUsageCriteria criteria)
  {
  }

  public: virtual Bool setShared(Provider const *provider, IdentifiableObject *parent,
                                 SharedPtr<IdentifiableObject> const &obj, Int &index) const;

  public: virtual Bool setPlain(Provider const *provider, IdentifiableObject *parent,
                                IdentifiableObject *obj, Int &index) const;

  public: virtual Bool remove(Provider const *provider, IdentifiableObject *parent, Int &index) const;

  public: virtual Bool getShared(Provider const *provider, IdentifiableObject const *parent,
                                 SharedPtr<IdentifiableObject> &result, Int &index) const;

  public: virtual Bool getPlain(Provider const *provider, IdentifiableObject const *parent,
                                IdentifiableObject *&result, Int &index) const;

}; // class

} } // namespace

#endif