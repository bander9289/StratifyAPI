//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved

#include "sgfx.hpp"
#include "ui/Element.hpp"
#include "draw.hpp"

using namespace ui;

Element::Element(){}

void Element::draw_scroll(const DrawingScaledAttr & attr,
		int selected,
		int total,
		int visible){
	Bitmap * b = attr.b();
	sg_point_t p = attr.p();
	Dim d = attr.d();
	int bar_size;
	sg_dim_t bar;
	bar_size = d.h() / total;
	b->set(p, d);
	p.y = p.y + selected*bar_size;
	bar.w = d.w();
	bar.h = bar_size;
	b->clear(p, bar);
}

Element * Element::event_handler(int event, const DrawingAttr & attr){
	switch(event){
	case UPDATE:
		return 0;
	}

	return this;
}

void Element::set_animation_type(u8 v){}
u8 Element::animation_type() const{ return AnimationAttr::PUSH_LEFT; }
void Element::set_animation_path(u8 path){}
void Element::set_animation(u8 type, u8 path){ set_animation_type(type); set_animation_path(path); }
u8 Element::animation_path() const { return AnimationAttr::SQUARED; }