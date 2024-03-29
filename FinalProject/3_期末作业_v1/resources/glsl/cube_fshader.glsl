#version 330 core

in vec3 frag_surface_normal_color;

in vec2 frag_uv;  // 纹理坐标
uniform sampler2D ground_texture;  // 草地纹理


uniform sampler2D shadow_buffer_tex;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 light_position;

in float red;
in float green;
in float blue;
out vec4 color;

in vec3 frag_position;
in vec3 frag_normal;
in vec4 shadow_coord;

uniform uint window_width;
uniform uint window_height;
uniform uint shadow_mapping_effect;

uniform vec3 shape_color;

uniform uint shadow_buffer_tex_size;

float get_shadow_val(float offset_x, float offset_y);

float get_shadow_val_lerp(float offset_x, float offset_y);

float get_shadow_val_pcf(bool lerp, int size);
float get_shadow_val_pcf_optim(bool lerp, int size);

void main(){

   float shadow = 1;
   
   if(shadow_mapping_effect == 1u){
      shadow = get_shadow_val(0.0, 0.0);
   }
   else if(shadow_mapping_effect == 2u){ //interp
      shadow = get_shadow_val_lerp(0.0, 0.0);
   }
   else if(shadow_mapping_effect == 3u){
      shadow = get_shadow_val_pcf(false, 1);
   }
   else if(shadow_mapping_effect == 4u){
      shadow = get_shadow_val_pcf(true, 1);
   }
   else if(shadow_mapping_effect == 5u){
      shadow = get_shadow_val_pcf(true, 5);
   }
   else if(shadow_mapping_effect == 6u){
      shadow = get_shadow_val_pcf_optim(true, 1);
   }
   //shadow = get_shadow_val_lerp(0.0,0.0);

   //color = vec4(frag_surface_normal_color*shadow, 1.0);


   vec3 light_dir = normalize(light_position-frag_position);
   float diffuse_light = dot(frag_normal, light_dir);
   //color = vec4(diffuse_light*shape_color, 1.0);
   vec4 texture_col = texture(ground_texture, frag_uv);
   vec3 texture_col_rgb = texture_col.rgb;
   color = vec4(diffuse_light*shadow*texture_col_rgb, 1.0);
}

float get_shadow_val_pcf(bool lerp, int size){
   float ret_val = 0.0;
   float size_increment = 1.0/float(shadow_buffer_tex_size);
   int counter_avg = 0;

   for(int i = -size; i <= size; i++){
      for(int j = -size; j <= size; j++){
         if(lerp){
            ret_val += get_shadow_val_lerp(i*size_increment, j*size_increment);
         }
         else{
            ret_val += get_shadow_val(i*size_increment, j*size_increment);
         }
         counter_avg++;
      }
   }

   ret_val /= counter_avg;

   return ret_val;
}

float get_shadow_val_pcf_optim(bool lerp, int size){
   float ret_val = 0.0;
   float size_increment = 1.0/float(shadow_buffer_tex_size);
   int counter_avg = 0;

   for(int i = -size; i <= size; i++){
      for(int j = -size; j <= size; j++){
         if(lerp){
            ret_val += get_shadow_val_lerp(i*size_increment*2, j*size_increment*2);
         }
         else{
            ret_val += get_shadow_val(i*size_increment*2, j*size_increment*2);
         }
         counter_avg++;
      }
   }

   ret_val /= counter_avg;

   return ret_val;
}

float get_shadow_val_lerp(float offset_x, float offset_y){
   float val = 0.0;
   float size_increment = 1.0/float(shadow_buffer_tex_size);
   vec2 shadow_tex_position = (shadow_coord.xy/shadow_coord.w) + vec2(offset_x, offset_y);
   //vec2 frac_parts = mod(shadow_tex_position*shadow_buffer_tex_size, 1.0);
   vec2 frac_parts = shadow_tex_position*shadow_buffer_tex_size-floor(shadow_tex_position*shadow_buffer_tex_size);

   float shadow00 = get_shadow_val(0.0+offset_x, 0.0+offset_y);
   float shadow01 = get_shadow_val(0.0+offset_x, size_increment+offset_y);
   float shadow10 = get_shadow_val(size_increment+offset_x, 0.0+offset_y);
   float shadow11 = get_shadow_val(size_increment+offset_x, size_increment+offset_y);

   float shadow0 = (1.0-frac_parts.x)*shadow00+frac_parts.x*shadow10;
   float shadow1 = (1.0-frac_parts.x)*shadow01+frac_parts.x*shadow11;

   val = (1.0-frac_parts.y)*shadow0+frac_parts.y*shadow1;

   return val;
}

float get_shadow_val(float offset_x, float offset_y){

   float shadow = 1.0;
   vec2 offset = vec2(offset_x, offset_y);

   float BIAS = 0.002;

   if ( texture( shadow_buffer_tex, (shadow_coord.xy/shadow_coord.w)+offset ).x  <  shadow_coord.z/shadow_coord.w-BIAS){
       shadow = 0.0;
   }
   return shadow;
}
