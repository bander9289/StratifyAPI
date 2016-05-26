//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved


#include <stdlib.h>

#include "draw/MBitmap.hpp"
#include "calc/Rle.hpp"
#include "sys/File.hpp"

using namespace draw;
using namespace sys;
using namespace calc;


void MBitmap::calc_members(mg_size_t w, mg_size_t h){
	m_bmap.columns = mg_calc_byte_width(w);
	m_bmap.dim.w = w;
	m_bmap.dim.h = h;
	m_bmap.data = (mg_bitmap_t*)data_const();
}

void MBitmap::init_members(){
	m_bmap.margin_bottom_right.dim = 0;
	m_bmap.margin_top_left.dim = 0;
}


void MBitmap::set_data(mg_bitmap_t * mem, mg_size_t w, mg_size_t h, bool readonly){
	Data::set(mem, calc_size(w,h), readonly);
	calc_members(w,h);
}

void MBitmap::set_data(mg_bitmap_hdr_t * hdr, bool readonly){
	char * ptr;
	ptr = (char*)hdr;
	ptr += sizeof(mg_bitmap_hdr_t);
	Data::set(ptr, calc_size(hdr->w, hdr->h), readonly);
	calc_members(hdr->w, hdr->h);
}

int MBitmap::alloc(mg_size_t w, mg_size_t h){
	if( Data::alloc(calc_size(w,h)) < 0 ){
		return -1;
	}
	calc_members(w,h);
	return 0;
}

void MBitmap::free(){
	if( Data::free() == 0 ){
		calc_members(0, 0);
	}
}

MBitmap::MBitmap(){
	init_members();
	calc_members(0, 0);
}

MBitmap::MBitmap(mg_size_t w, mg_size_t h){
	init_members();
	alloc(w,h);
}


MBitmap::MBitmap(mg_bitmap_t * mem, mg_size_t w, mg_size_t h, bool readonly){
	init_members();
	set_data(mem, w, h, readonly);
}

MBitmap::MBitmap(mg_bitmap_hdr_t * hdr, bool readonly){
	init_members();
	set_data(hdr, readonly);
}

MBitmap::~MBitmap(){
	free();
}

int MBitmap::calc_byte_width(int w){
	return (w + 7) >> 3;
}

int MBitmap::calc_word_width(int w){
	return (w + 31) / 32;
}

mg_point_t MBitmap::calc_center() const{
	mg_point_t p;
	p.x = w()/2;
	p.y = h()/2;
	return p;
}

int MBitmap::set_size(mg_size_t w, mg_size_t h, mg_size_t offset){
	if( calc_size(w,h) <= capacity() ){
		m_bmap.dim.w = w;
		m_bmap.dim.h = h;
		m_bmap.columns = mg_calc_byte_width(w);
		return 0;
	}
	return -1;
}

mg_bitmap_t * MBitmap::data(mg_point_t p) const {

	if( data() == 0 ){
		return 0;
	}

	return mg_data(bmap_const(),p);
}

mg_bitmap_t * MBitmap::data(mg_int_t x, mg_int_t y) const{
	return mg_data(bmap_const(), mg_point(x,y));
}

const mg_bitmap_t * MBitmap::data_const(mg_point_t p) const {
	if( data_const() == 0 ){
		return 0;
	}

	return data_const() + p.x / 8 + p.y * m_bmap.columns;
}



int MBitmap::load(const char * path){
	mg_bitmap_hdr_t hdr;
	File f;
	void * src;



	if( f.open(path, File::READONLY) < 0 ){
		return -1;
	}

	if( f.read(&hdr, sizeof(hdr)) != sizeof(hdr) ){
		f.close();
		return -1;
	}

	if( set_size(hdr.w, hdr.h) < 0 ){
		//couln't resize using existing memory -- try resizing
		if( alloc(hdr.w, hdr.h) < 0 ){
			f.close();
			return -1;
		}
	}

	src = data();


	if( f.read(src, hdr.size) != (ssize_t)hdr.size ){
		f.close();
		return -1;
	}

	if( f.close() < 0 ){
		return -1;
	}

	return 0;
}


MDim MBitmap::load_dim(const char * path){
	mg_bitmap_hdr_t hdr;
	File f;
	if( f.open(path, File::READONLY) < 0 ){
		return MDim();
	}

	if( f.read(&hdr, sizeof(hdr)) != sizeof(hdr) ){
		f.close();
		return MDim();
	}

	f.close();
	return MDim(hdr.w, hdr.h);
}

int MBitmap::load(const char * path, mg_point_t p){
	mg_bitmap_hdr_t hdr;
	File f;
	void * src;
	mg_int_t j;
	size_t w;

	if( f.open(path, File::READONLY) < 0 ){
		return -1;
	}

	if( f.read(&hdr, sizeof(hdr)) != sizeof(hdr) ){
		f.close();
		return -1;
	}

	//see if bitmap will fit

	w = calc_byte_width(hdr.w);
	if( (int)w > (columns() - p.x/8) ){
		w = columns() - p.x/8;
	}

	for(j=0; (j < hdr.h) && (p.y+j < h()); j++){
		src = data(mg_point(p.x,p.y+j));
		if( f.read(src, w) != (int)w ){
			f.close();
			return -1;
		}
	}

	if( f.close() < 0 ){
		return -1;
	}

	return 0;
}

int MBitmap::save(const char * path) const{
	mg_bitmap_hdr_t hdr;

	hdr.w = w();
	hdr.h = h();
	hdr.size = calc_size();

	File f;
	if( f.create(path, true) < 0 ){
		return -1;
	}


	if( f.write(&hdr, sizeof(hdr)) < 0 ){
		f.close();
		unlink(path);
		return -1;
	}

	if( f.write(data(), hdr.size) != (ssize_t)hdr.size ){
		f.close();
		unlink(path);
		return -1;
	}


	if( f.close() < 0 ){
		return -1;
	}

	return 0;
}


int MBitmap::set_bitmap_column(const MBitmap * bitmap, mg_point_t p, mg_int_t col){
	return set_bitmap_column(bitmap, p, col, bitmap->h());
}

int MBitmap::set_bitmap_column(const MBitmap * bitmap, mg_point_t p, mg_int_t col, mg_size_t h){
	mg_point_t i;
	if( (bitmap == 0) || (data() == 0) ){
		return -1;
	}

	if( col >= bitmap->w() ){
		return 0; //nothing to do
	}

	i.x = col;
	for(i.y=0; i.y < h; i.y++){
		if( bitmap->tst_pixel(i) ){
			set_pixel(p);
		} else {
			clr_pixel(p);
		}
		p.y++;
	}

	return 0;
}

/*
bool MBitmap::tst_pixel(mg_point_t p) const {
	return mg_tst_pixel(bmap_const(), p) != 0;
}

void MBitmap::set_pixel(mg_point_t p){
	mg_set_pixel(bmap(), p);
}

void MBitmap::inv_pixel(mg_point_t p){
	mg_inv_pixel(bmap(), p);
}
*/

void MBitmap::invert(){
	mg_inv_area(bmap(), mg_point_origin(), dim().dim(), 0xFF);
}

void MBitmap::invert(mg_point_t p, mg_dim_t d, mg_bitmap_t v){
	mg_inv_area(bmap(), p, d, v);
}

void MBitmap::fill(mg_bitmap_t v, mg_int_t start, mg_size_t h){
	mg_fill(bmap(), v, start, h);
}

void MBitmap::fill(mg_bitmap_t v){
	memset(data(), v, calc_size());
}


void MBitmap::set_vline(mg_int_t x, mg_int_t ymin, mg_int_t ymax, mg_size_t thickness){
	mg_set_vline(bmap(),x,ymin,ymax,thickness);
}

void MBitmap::set_hline(mg_int_t xmin, mg_int_t xmax, mg_int_t y, mg_size_t thickness){
	mg_set_hline(bmap(),xmin,xmax,y,thickness);
}

void MBitmap::clr_vline(mg_int_t x, mg_int_t ymin, mg_int_t ymax, mg_size_t thickness){
	mg_clr_vline(bmap(),x,ymin,ymax,thickness);
}

void MBitmap::clr_hline(mg_int_t xmin, mg_int_t xmax, mg_int_t y, mg_size_t thickness){
	mg_clr_hline(bmap(),xmin,xmax,y,thickness);
}

void MBitmap::clr_line(mg_point_t p1, mg_point_t p2, mg_size_t thickness){
	mg_clr_line(bmap(), p1, p2, thickness);
}


void MBitmap::set_line(mg_point_t p1, mg_point_t p2, mg_size_t thickness){
	mg_set_line(bmap(), p1, p2, thickness);
}

void MBitmap::pour(mg_point_t p){
	mg_pour(bmap(), p);
}


void MBitmap::shift_right(int count){
	shift_right(count, h());
}

void MBitmap::shift_right(int count, mg_size_t h){
	//mg_shift_right(bmap(), count, 0, h);
}

void MBitmap::shift_left(int count){
	shift_left(count, h());
}

void MBitmap::shift_left(int count, mg_size_t h){
	//mg_shift_left(bmap(), count, 0, h);
}


void MBitmap::show() const{
	mg_show(bmap_const());
}

void MBitmap::flip_x(){
	mg_flip_x(bmap());
}

void MBitmap::flip_y(){
	mg_flip_y(bmap());
}

void MBitmap::flip_xy(){
	mg_flip_xy(bmap());
}




