/*
 * Copyright (c) 2012-2013 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "class_type.h"
# include  "compile.h"
# include  "vpi_priv.h"
# include  "config.h"
# include  <map>
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cassert>

using namespace std;

/*
 * This class_property_t class is an abstract base class for
 * representing a property of an instance. The definition keeps and
 * array (of pointers) of these in order to define the the class.
 */
class class_property_t {
    public:
      explicit inline class_property_t() { }
      virtual ~class_property_t() =0;
	// How much space does an instance of this property require?
      virtual size_t instance_size() const =0;

      void set_offset(size_t off) { offset_ = off; }

    public:
      virtual void construct(char*buf) const;
      virtual void destruct(char*buf) const;

      virtual void set_vec4(char*buf, const vvp_vector4_t&val);
      virtual void get_vec4(char*buf, vvp_vector4_t&val);

    protected:
      size_t offset_;
};

class_property_t::~class_property_t()
{
}

void class_property_t::construct(char*) const
{
}

void class_property_t::destruct(char*) const
{
}

void class_property_t::set_vec4(char*, const vvp_vector4_t&)
{
      assert(0);
}

void class_property_t::get_vec4(char*, vvp_vector4_t&)
{
      assert(0);
}

/*
 */
template <class T> class property_atom : public class_property_t {
    public:
      inline explicit property_atom(void) { }
      ~property_atom() { }

      size_t instance_size() const { return sizeof(T); }

    public:
      void construct(char*buf) const
      { T*tmp = reinterpret_cast<T*> (buf+offset_);
	*tmp = 0;
      }

      void set_vec4(char*buf, const vvp_vector4_t&val)
      {
	    T*tmp = reinterpret_cast<T*> (buf+offset_);
	    bool flag = vector4_to_value(val, *tmp, true, false);
	    assert(flag);
      }

      void get_vec4(char*buf, vvp_vector4_t&val)
      {
	    T*src = reinterpret_cast<T*> (buf+offset_);
	    const size_t tmp_cnt = (sizeof(T) + sizeof(unsigned long)-1)/sizeof(unsigned long);
	    unsigned long tmp[tmp_cnt];
	    tmp[0] = src[0];
	    for (size_t idx = 1 ; idx < tmp_cnt ; idx += 1)
		  tmp[idx] = 0;

	    val.resize(8*sizeof(T));
	    val.setarray(0, val.size(), tmp);
      }
};

/* **** */

class_type::class_type(const string&nam, size_t nprop)
: class_name_(nam), properties_(nprop)
{
      instance_size_ = 0;
}

void class_type::set_property(size_t idx, const string&name, const string&type)
{
      assert(idx < properties_.size());
      properties_[idx].name = name;

      if (type == "b8")
	    properties_[idx].type = new property_atom<uint8_t>;
      else if (type == "b16")
	    properties_[idx].type = new property_atom<uint16_t>;
      else if (type == "b32")
	    properties_[idx].type = new property_atom<uint32_t>;
      else if (type == "b64")
	    properties_[idx].type = new property_atom<uint64_t>;
      else if (type == "sb8")
	    properties_[idx].type = new property_atom<int8_t>;
      else if (type == "sb16")
	    properties_[idx].type = new property_atom<int16_t>;
      else if (type == "sb32")
	    properties_[idx].type = new property_atom<int32_t>;
      else if (type == "sb64")
	    properties_[idx].type = new property_atom<int64_t>;
      else
	    properties_[idx].type = 0;
}

void class_type::finish_setup(void)
{
      map<size_t, vector<size_t> > size_map;
	// Add up all the sizes to get a total instance size. This
	// figures out how much memory a complete instance will need.
      size_t accum = 0;
      for (size_t idx = 0 ; idx < properties_.size() ; idx += 1) {
	    size_t instance_size = properties_[idx].type->instance_size();
	    accum += instance_size;
	    size_map[instance_size].push_back(idx);
      }

      instance_size_ = accum;

	// Now allocate the properties to offsets within an instance
	// space. Allocate the properties largest objects first so
	// that they are assured better alignment.
      accum = 0;
      for (map<size_t, vector<size_t> >::reverse_iterator cur = size_map.rbegin()
		 ; cur != size_map.rend() ; ++ cur) {
	    for (size_t idx = 0 ; idx < cur->second.size() ; idx += 1) {
		  size_t pid = cur->second[idx];
		  class_property_t*ptype = properties_[pid].type;
		  assert(ptype->instance_size() == cur->first);
		  ptype->set_offset(accum);
		  accum += cur->first;
	    }
      }
}

class_type::inst_t class_type::instance_new() const
{
      char*buf = new char [instance_size_];

      for (size_t idx = 0 ; idx < properties_.size() ; idx += 1)
	    properties_[idx].type->construct(buf);

      return reinterpret_cast<inst_t> (buf);
}

void class_type::instance_delete(class_type::inst_t obj) const
{
      char*buf = reinterpret_cast<char*> (obj);

      for (size_t idx = 0 ; idx < properties_.size() ; idx += 1)
	    properties_[idx].type->destruct(buf);

      delete[]buf;
}

void class_type::set_vec4(class_type::inst_t obj, size_t pid,
			  const vvp_vector4_t&val) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->set_vec4(buf, val);
}

void class_type::get_vec4(class_type::inst_t obj, size_t pid,
			  vvp_vector4_t&val) const
{
      char*buf = reinterpret_cast<char*> (obj);
      assert(pid < properties_.size());
      properties_[pid].type->get_vec4(buf, val);
}

int class_type::get_type_code(void) const
{
      return vpiClassDefn;
}

static class_type*compile_class = 0;

void compile_class_start(char*lab, char*nam, unsigned ntype)
{
      assert(compile_class == 0);
      compile_class = new class_type(nam, ntype);
      compile_vpi_symbol(lab, compile_class);
      free(lab);
      delete[]nam;
}

void compile_class_property(unsigned idx, char*nam, char*typ)
{
      assert(compile_class);
      compile_class->set_property(idx, nam, typ);
      delete[]nam;
      delete[]typ;
}

void compile_class_done(void)
{
      struct __vpiScope*scope = vpip_peek_current_scope();
      assert(scope);
      assert(compile_class);
      compile_class->finish_setup();
      scope->classes[compile_class->class_name()] = compile_class;
      compile_class = 0;
}

#ifdef CHECK_WITH_VALGRIND
void class_def_delete(class_type *item)
{
      delete item;
}
#endif
