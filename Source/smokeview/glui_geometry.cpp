#define CPP
#include "options.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include GLUT_H

#include "smokeviewvars.h"

//*** geomprocinfo entries
#define STRUCTURED_ROLLOUT     0
#define UNSTRUCTURED_ROLLOUT   1
#define IMMERSED_DIAGNOSTICS   2
#define HVAC_ROLLOUT           3
#define TERRAIN_ROLLOUT        4

procdata  geomprocinfo[5];
int      ngeomprocinfo = 0;

#define XMIN_SPIN             20
#define YMIN_SPIN             21
#define ZMIN_SPIN             22
#define XMAX_SPIN             23
#define YMAX_SPIN             24
#define ZMAX_SPIN             25
#define UPDATE_LIST           31
#define RADIO_WALL            32
#define SAVE_SETTINGS_GEOM    33
#define VISAXISLABELS         34
#define GEOM_IVECFACTOR       38
#define SHOW_TEXTURE_1D_IMAGE 40
#define TERRAIN_ZMIN          41
#define TERRAIN_ZMAX          42
#define RESET_ZBOUNDS         43
#define TERRAIN_ZLEVEL        44
#define SHOW_ZLEVEL           45
#define GEOM_VERT_EXAG        46
#define UPDATE_GEOM           48
#define SURF_SET              49
#define SURF_GET              50
#define SHOWONLY_TOP          51
#define GEOM_FDS_DOMAIN       52
#define GEOM_OUTLINECOLOR     53
#ifdef pp_DECIMATE
#define GEOM_DECIMATE         54
#define GEOM_DECIMATE_DELTA   55
#endif

#define HVAC_PROPS            -1
#define HVAC_SHOWALL_NETWORK  -2
#define HVAC_HIDEALL_NETWORK  -3
#define HVAC_SHOW_DUCT_LABELS -4
#define HVAC_SHOW_COMPONENTS  -5
#define HVAC_SHOW_NODE_LABELS -6
#define HVAC_SHOW_FILTERS     -7
#define HVAC_METRO_VIEW       -8
#define HVAC_SHOWALL_CONNECTIONS  -12
#define HVAC_HIDEALL_CONNECTIONS  -13
#define HVAC_SHOW_NETWORKS     -15
#define HVAC_SHOW_CONNECTIONS  -16
#define HVAC_DUCTNODE_NETWORK  -17
#define HVAC_CELL_VIEW         -18
#define HVAC_NODE_LIST         -19
#define HVAC_DUCT_LIST         -20
#define HVACDUCT_SET_BOUNDS    -21
#define HVACNODE_SET_BOUNDS    -22

#define TERRAIN_TYPE      0
#define TERRAIN_TOP_ONLY  1

GLUI_Checkbox *CHECKBOX_terrain_top_surface    = NULL;
GLUI_Checkbox **CHECKBOX_hvac_show_networks    = NULL;
GLUI_Checkbox **CHECKBOX_hvac_show_connections = NULL;
GLUI_Checkbox *CHECKBOX_hvac_show_connection   = NULL;
GLUI_Checkbox *CHECKBOX_hvac_show_network      = NULL;
GLUI_Checkbox *CHECKBOX_hvac_show_duct_labels  = NULL;
GLUI_Checkbox *CHECKBOX_hvac_show_node_labels  = NULL;
GLUI_Checkbox* CHECKBOX_hvac_metro_view        = NULL;
GLUI_Checkbox *CHECKBOX_hvac_cell_view         = NULL;

GLUI_Checkbox *CHECKBOX_showgeom_inside_domain  = NULL;
GLUI_Checkbox *CHECKBOX_showgeom_outside_domain = NULL;
GLUI_Checkbox **CHECKBOX_terrain_texture_show   = NULL;
GLUI_Checkbox *CHECKBOX_cfaces = NULL;
GLUI_Checkbox *CHECKBOX_show_cface_normals = NULL;
GLUI_Checkbox *CHECKBOX_show_zlevel = NULL;
GLUI_Checkbox *CHECKBOX_surface_solid=NULL, *CHECKBOX_surface_outline=NULL, *CHECKBOX_surface_points = NULL;
GLUI_Checkbox *CHECKBOX_geom_force_transparent = NULL;
GLUI_Checkbox *CHECKBOX_geomtest=NULL, *CHECKBOX_triangletest=NULL;
GLUI_Checkbox *CHECKBOX_show_geom_normal = NULL;
GLUI_Checkbox *CHECKBOX_smooth_geom_normal = NULL;
GLUI_Checkbox *CHECKBOX_show_texture_1dimage = NULL;
GLUI_Checkbox *CHECKBOX_showonly_top = NULL;
#ifdef pp_DECIMATE
GLUI_Checkbox *CHECKBOX_use_decimate_geom = NULL;
#endif

GLUI_RadioGroup *RADIO_terrain_type = NULL;
GLUI_RadioGroup *RADIO_select_geom = NULL;
GLUI_RadioGroup *RADIO_cface_type = NULL;
GLUI_RadioGroup *RADIO_show_geom_boundingbox = NULL;
GLUI_RadioGroup *RADIO_hvac_show_component_labels = NULL;
GLUI_RadioGroup *RADIO_hvac_show_filters          = NULL;


GLUI_StaticText *STATIC_vertx1=NULL;
GLUI_StaticText *STATIC_verty1=NULL;
GLUI_StaticText *STATIC_vertz1=NULL;
GLUI_StaticText *STATIC_vertx2 = NULL;
GLUI_StaticText *STATIC_verty2 = NULL;
GLUI_StaticText *STATIC_vertz2 = NULL;
GLUI_StaticText *STATIC_dist=NULL;
GLUI_StaticText *STATIC_tri_area = NULL;

GLUI_Checkbox *CHECKBOX_use_surf_color=NULL;

GLUI_Rollout *ROLLOUT_geomtest=NULL;
GLUI_Rollout *ROLLOUT_geom_rgbs = NULL;
GLUI_Rollout *ROLLOUT_geom_properties=NULL;

GLUI_Panel *PANEL_hvac_options     = NULL;
GLUI_Panel *PANEL_hvac_filter      = NULL;
GLUI_Panel *PANEL_hvac_components  = NULL;
GLUI_Panel *PANEL_hvac_duct        = NULL;
GLUI_Panel *PANEL_hvac_node        = NULL;
GLUI_Panel *PANEL_hvac_network     = NULL;
GLUI_Panel *PANEL_hvac_connections = NULL;
GLUI_Panel *PANEL_hvac_group1 = NULL;
GLUI_Panel *PANEL_hvac_group2 = NULL;

GLUI_Panel *PANEL_outlinecolor=NULL;
GLUI_Panel *PANEL_surf_color = NULL;
GLUI_Panel *PANEL_surf_axis = NULL;
GLUI_Panel *PANEL_surf_coloraxis = NULL;
GLUI_Panel *PANEL_face_cface = NULL;
GLUI_Panel *PANEL_elevation_color = NULL;
GLUI_Panel *PANEL_geom_offset_outline = NULL;
GLUI_Panel *PANEL_geom_close = NULL;
GLUI_Panel *PANEL_geom_transparency = NULL;
GLUI_Panel *PANEL_normals = NULL;

GLUI_Spinner *SPINNER_terrain_skip        = NULL;
GLUI_Spinner *SPINNER_slice_skip2         = NULL;
GLUI_Spinner *SPINNER_geom_transparency   = NULL;
GLUI_Spinner *SPINNER_hvac_duct_width     = NULL;
GLUI_Spinner *SPINNER_hvac_node_size      = NULL;
GLUI_Spinner *SPINNER_hvac_cell_node_size = NULL;
GLUI_Spinner *SPINNER_hvac_component_size = NULL;
GLUI_Spinner *SPINNER_hvac_filter_size    = NULL;
GLUI_Spinner *SPINNER_hvac_duct_color[3];
GLUI_Spinner *SPINNER_hvac_node_color[3];

GLUI_Spinner *SPINNER_outlinecolor_red = NULL;
GLUI_Spinner *SPINNER_outlinecolor_green = NULL;
GLUI_Spinner *SPINNER_outlinecolor_blue = NULL;
GLUI_Spinner *SPINNER_geom_ivecfactor = NULL;
GLUI_Spinner *SPINNER_geom_vert_exag=NULL;
GLUI_Spinner *SPINNER_geom_zmin = NULL, *SPINNER_geom_zmax = NULL, *SPINNER_geom_zlevel=NULL;
GLUI_Spinner *SPINNER_geom_vertex1_rgb[3]  = {NULL, NULL, NULL};
GLUI_Spinner *SPINNER_geom_vertex2_rgb[3]  = {NULL, NULL, NULL};
GLUI_Spinner *SPINNER_geom_triangle_rgb[3] = {NULL, NULL, NULL};
GLUI_Spinner *SPINNER_surf_rgb[3]          = {NULL, NULL, NULL};
GLUI_Spinner *SPINNER_surf_axis[3]         = {NULL, NULL, NULL};
#ifdef pp_DECIMATE
GLUI_Spinner *SPINNER_terrain_deimate_delta=NULL;
#endif

#define VOL_SHOWHIDE           3
#define SELECT_GEOM            4
#define VOL_USE_CFACES         5
#define GEOM_BOUNDING_BOX      6

GLUI *glui_geometry=NULL;

GLUI_Button *BUTTON_blockage_1=NULL;
GLUI_Button *BUTTON_reset_zbounds = NULL;
GLUI_Button *BUTTON_reset_offset = NULL;

GLUI_Checkbox *CHECKBOX_blockage=NULL;

GLUI_EditText *EDIT_xmin=NULL, *EDIT_ymin=NULL, *EDIT_zmin=NULL;
GLUI_EditText *EDIT_xmax=NULL, *EDIT_ymax=NULL, *EDIT_zmax=NULL;

GLUI_Listbox *LIST_obst_surface[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
GLUI_Listbox *LIST_geom_surface=NULL;
GLUI_Listbox *LIST_hvac_network_ductnode_index = NULL;
GLUI_Listbox *LIST_hvacnodevar_index = NULL;
GLUI_Listbox *LIST_hvacductvar_index = NULL;

GLUI_Panel *PANEL_geomtest2 = NULL;
GLUI_Panel *PANEL_cfaces = NULL;
GLUI_Panel *PANEL_obj_select=NULL,*PANEL_faces=NULL,*PANEL_triangles=NULL,*PANEL_volumes=NULL,*PANEL_geom_showhide;
GLUI_Panel *PANEL_boundingbox = NULL;
GLUI_Panel *PANEL_vertex1_rgb = NULL;
GLUI_Panel *PANEL_vertex2_rgb = NULL;
GLUI_Panel *PANEL_triangle_rgb = NULL;
GLUI_Panel *PANEL_properties_surf = NULL;
GLUI_Panel *PANEL_properties_triangle = NULL;
GLUI_Panel *PANEL_properties_vertex = NULL;
GLUI_Panel *PANEL_properties2 = NULL;
GLUI_Panel *PANEL_obj_stretch2=NULL,*PANEL_obj_stretch3=NULL, *PANEL_obj_stretch4=NULL;
GLUI_Panel *PANEL_geomedgecheck=NULL;
GLUI_Panel *PANEL_group1=NULL;
GLUI_Panel *PANEL_geom_offset=NULL;
GLUI_Panel *PANEL_terrain_images = NULL;
#ifdef pp_DECIMATE
GLUI_Panel *PANEL_terrain_decimate = NULL;
GLUI_Panel *PANEL_terrain_decimate_sizes = NULL;
#endif
GLUI_Panel *PANEL_geom_show = NULL;

GLUI_Rollout *ROLLOUT_hvac = NULL;
GLUI_Rollout *ROLLOUT_structured=NULL;
GLUI_Rollout *ROLLOUT_unstructured=NULL;
GLUI_Rollout *ROLLOUT_terrain = NULL;

GLUI_Spinner *SPINNER_face_factor=NULL;

#ifdef pp_DECIMATE
GLUI_StaticText *STATIC_terrain_pixel_size=NULL;
GLUI_StaticText *STATIC_terrain_cell_size=NULL;
GLUI_StaticText *STATIC_terrain_geom_size=NULL;
#endif
GLUI_StaticText *STATIC_blockage_index=NULL;
GLUI_StaticText *STATIC_mesh_index=NULL;
GLUI_StaticText *STATIC_label=NULL;
GLUI_StaticText *STATIC_id_label = NULL;

char a_updatelabel[1000];
char *updatelabel=NULL;


/* ------------------ UpdateHVACViews ------------------------ */

extern "C" void UpdateHVACViews(void){
  CHECKBOX_hvac_metro_view->set_int_val(hvac_metro_view);
  CHECKBOX_hvac_cell_view->set_int_val(hvac_cell_view);
}

  /* ------------------ UpdateTerrainTexture ------------------------ */

extern "C" void UpdateTerrainTexture(int val){
  if(CHECKBOX_terrain_texture_show!=NULL&&val>=0&&val<nterrain_textures){
    texturedata *texti;

    texti = terrain_textures+val;
    if(texti->loaded==1&&CHECKBOX_terrain_texture_show[val]!=NULL){
      CHECKBOX_terrain_texture_show[val]->set_int_val(texti->display);
    }
  }
}

/* ------------------ TerrainTextureCB ------------------------ */

void TerrainTextureCB(int val){
  updatemenu = 1;
}

/* ------------------ GeomRolloutCB ------------------------ */

void GeomRolloutCB(int var){
  ToggleRollout(geomprocinfo, ngeomprocinfo, var);
}

/* ------------------ UpdateSelectGeom ------------------------ */

extern "C" void UpdateSelectGeom(void){
  RADIO_select_geom->set_int_val(select_geom);
}

/* ------------------ UpdateShowOnlyTop ------------------------ */

extern "C" void UpdateShowOnlyTop(void){
  if(CHECKBOX_showonly_top!=NULL)CHECKBOX_showonly_top->set_int_val(terrain_showonly_top);
}

/* ------------------ UpdateWhereFaceVolumes ------------------------ */

extern "C" void UpdateWhereFaceVolumes(void){
  if(CHECKBOX_showgeom_inside_domain!=NULL)CHECKBOX_showgeom_inside_domain->set_int_val(showgeom_inside_domain);
  if(CHECKBOX_showgeom_outside_domain!=NULL)CHECKBOX_showgeom_outside_domain->set_int_val(showgeom_outside_domain);
}

/* ------------------ UpdateGluiCfaces ------------------------ */

extern "C" void UpdateGluiCfaces(void){
  glui_use_cfaces = use_cfaces;
  if(CHECKBOX_cfaces!=NULL){
    CHECKBOX_cfaces->set_int_val(use_cfaces);
  }
  if(CHECKBOX_show_cface_normals!=NULL)CHECKBOX_show_cface_normals->set_int_val(show_cface_normals);
}

/* ------------------ UpdateGeomBoundingBox ------------------------ */

extern "C" void UpdateGeomBoundingBox(void){
  if(RADIO_show_geom_boundingbox!=NULL)RADIO_show_geom_boundingbox->set_int_val(show_geom_boundingbox);
}

/* ------------------ UpdateGeometryControls ------------------------ */

extern "C" void UpdateGeometryControls(void){
  if(CHECKBOX_surface_solid!=NULL)CHECKBOX_surface_solid->set_int_val(show_faces_shaded);
  if(CHECKBOX_surface_outline!=NULL)CHECKBOX_surface_outline->set_int_val(show_faces_outline);

  if(CHECKBOX_show_geom_normal != NULL)CHECKBOX_show_geom_normal->set_int_val(show_geom_normal);
  if(CHECKBOX_smooth_geom_normal != NULL)CHECKBOX_smooth_geom_normal->set_int_val(smooth_geom_normal);
}

/* ------------------ GetGeomDialogState ------------------------ */

extern "C" void GetGeomDialogState(void){
  if(ROLLOUT_structured!=NULL){
    if(ROLLOUT_structured->is_open){
      structured_isopen=1;
    }
    else{
      structured_isopen=0;
    }
  }
  if(ROLLOUT_unstructured!=NULL){
    if(ROLLOUT_unstructured->is_open){
      unstructured_isopen=1;
    }
    else{
      unstructured_isopen=0;
    }
  }
}

/* ------------------ HaveTexture ------------------------ */

int HaveTexture(void){
  int i;

  for(i = 0; i < ntextureinfo; i++){
    texturedata *texti;

    texti = textureinfo + i;
    if(texti->loaded == 1 && texti->used == 1)return 1;
  }
  return 0;
}

/* ------------------ BlockeditDlgCB ------------------------ */

void BlockeditDlgCB(int var){
  switch(var){
  case SAVE_SETTINGS_GEOM:
    updatemenu = 1;
    WriteIni(LOCAL_INI, NULL);
    break;
  case CLOSE_WINDOW:
    DialogMenu(DIALOG_GEOMETRY);
    break;
  default:
    assert(FFALSE);
    break;
  }

}

/* ------------------ UpdateTriangleInfo ------------------------ */


extern "C" void UpdateTriangleInfo(surfdata *tri_surf, float tri_area){
  char label[100];

  LIST_geom_surface->set_int_val(tri_surf->in_geom_list);

  //sprintf(label, "triangle area: %f m2", tri_area);
  snprintf(label, sizeof(label), "triangle area: %f m2", tri_area);
  STATIC_tri_area->set_name(label);
  VolumeCB(SURF_GET);
}

  /* ------------------ UpdateVertexInfo ------------------------ */

extern "C" void UpdateVertexInfo(float *xyz1, float *xyz2){
  char label[100];

  if(xyz1!=NULL){
//    sprintf(label, "x1: %f", xyz1[0]);
    snprintf(label, sizeof(label), "x1: %f", xyz1[0]);
    STATIC_vertx1->set_name(label);
//    sprintf(label, "y1: %f", xyz1[1]);
    snprintf(label, sizeof(label), "y1: %f", xyz1[1]);
    STATIC_verty1->set_name(label);
//    sprintf(label, "z1: %f", xyz1[2]);
    snprintf(label, sizeof(label), "z1: %f", xyz1[2]);
    STATIC_vertz1->set_name(label);
  }
  else{
    STATIC_vertx1->set_name("x1:");
    STATIC_verty1->set_name("y1:");
    STATIC_vertz1->set_name("z1:");
  }
  if(xyz2!=NULL){
//    sprintf(label, "x2: %f", xyz2[0]);
    snprintf(label, sizeof(label), "x2: %f", xyz2[0]);
    STATIC_vertx2->set_name(label);
//    sprintf(label, "y2: %f", xyz2[1]);
    snprintf(label, sizeof(label), "y2: %f", xyz2[1]);
    STATIC_verty2->set_name(label);
//    sprintf(label, "z2: %f", xyz2[2]);
    snprintf(label, sizeof(label), "z2: %f", xyz2[2]);
    STATIC_vertz2->set_name(label);
  }
  else{
    STATIC_vertx2->set_name("x2:");
    STATIC_verty2->set_name("y2:");
    STATIC_vertz2->set_name("z2:");
  }
  if(xyz1!=NULL&&xyz2!=NULL){
    float dx, dy, dz, dist;

    dx = xyz1[0]-xyz2[0];
    dy = xyz1[1]-xyz2[1];
    dz = xyz1[2]-xyz2[2];
    dist = sqrt(dx*dx+dy*dy+dz*dz);
//    sprintf(label, "dist: %f", dist);
    snprintf(label, sizeof(label), "dist: %f", dist);
    STATIC_dist->set_name(label);
  }
  else{
    STATIC_dist->set_name("dist:");
  }
}

/* ------------------ Glui2HVAC ------------------------ */

void Glui2HVAC(void){
  int i;

  for(i = 0;i < nhvacinfo;i++){
    if(hvac_network_ductnode_index==-1||hvac_network_ductnode_index==i){
      int display;
      hvacdata *hvaci;
      char *network_name;

      hvaci = hvacinfo + i;
      display      = hvaci->display;
      network_name = hvaci->network_name;
      memcpy(hvaci, glui_hvac, sizeof(hvacdata));
      hvaci->display      = display;
      hvaci->network_name = network_name;
    }
  }
}

/* ------------------ HVAC2Glui ------------------------ */

extern "C" void HVAC2Glui(int index){
  hvacdata *hvaci;
  int i;

  if(index >= nhvacinfo)return;
  if(index < 0){
    Glui2HVAC();
    return;
  }

  hvaci = hvacinfo + index;
  memcpy(glui_hvac, hvaci, sizeof(hvacdata));
  for(i=0;i<3;i++){
    SPINNER_hvac_duct_color[i]->set_int_val(glui_hvac->duct_color[i]);
    SPINNER_hvac_node_color[i]->set_int_val(glui_hvac->node_color[i]);
  }

  SPINNER_hvac_duct_width->set_float_val(glui_hvac->duct_width);
  RADIO_hvac_show_component_labels->set_int_val(glui_hvac->show_component);
  CHECKBOX_hvac_show_duct_labels->set_int_val(glui_hvac->show_duct_labels);

  SPINNER_hvac_filter_size->set_float_val(glui_hvac->filter_size);
  SPINNER_hvac_component_size->set_float_val(glui_hvac->component_size);
  SPINNER_hvac_node_size->set_float_val(glui_hvac->node_size);
  SPINNER_hvac_cell_node_size->set_float_val(glui_hvac->cell_node_size);
  CHECKBOX_hvac_show_node_labels->set_int_val(glui_hvac->show_node_labels);
  RADIO_hvac_show_filters->set_int_val(glui_hvac->show_filters);
}

/* ------------------ UpdateTerrainGlui ------------------------ */

extern "C" void UpdateTerrainGlui(void){
  if(CHECKBOX_terrain_top_surface!=NULL)CHECKBOX_terrain_top_surface->set_int_val(terrain_showonly_top);
  if(CHECKBOX_showonly_top != NULL)CHECKBOX_showonly_top->set_int_val(terrain_showonly_top);
  if(RADIO_terrain_type!=NULL)RADIO_terrain_type->set_int_val(visTerrainType);
}

/* ------------------ UpdateHVACVarLists ------------------------ */

extern "C" void UpdateHVACVarLists(void){
  if(LIST_hvacductvar_index!=NULL)LIST_hvacductvar_index->set_int_val(hvacductvar_index);
  if(LIST_hvacnodevar_index!=NULL)LIST_hvacnodevar_index->set_int_val(hvacnodevar_index);
}

/* ------------------ HvacCB ------------------------ */

extern "C" void HvacCB(int var){
  int i;

  updatemenu = 1;
  if(var>=0){
    return;
  }
  switch(var){
    case HVACDUCT_SET_BOUNDS:
      ShowBoundsDialog(DLG_HVACDUCT);
      break;
    case HVACNODE_SET_BOUNDS:
      ShowBoundsDialog(DLG_HVACNODE);
      break;
    case HVAC_NODE_LIST:
      if(hvacnodevar_index>=0){
        if(hvacnodevalsinfo->loaded == 0)ReadHVACData(LOAD);
        HVACNodeValueMenu(hvacnodevar_index);
      }
      break;
    case HVAC_DUCT_LIST:
      if(hvacductvar_index>=0){
        if(hvacductvalsinfo->loaded == 0)ReadHVACData(LOAD);
        HVACDuctValueMenu(hvacductvar_index);
      }
    break;
    case HVAC_DUCTNODE_NETWORK:
      HVAC2Glui(hvac_network_ductnode_index);
      break;
    case HVAC_SHOW_DUCT_LABELS:
    case HVAC_SHOW_COMPONENTS:
    case HVAC_SHOW_NODE_LABELS:
    case HVAC_SHOW_FILTERS:
    case HVAC_METRO_VIEW:
    case HVAC_CELL_VIEW:
      break;
    case HVAC_PROPS:
      if(glui_hvac->duct_width<1.0){
        glui_hvac->duct_width = 1.0;
        SPINNER_hvac_duct_width->set_float_val(glui_hvac->duct_width);
      }
      if(glui_hvac->component_size < 0.1){
        glui_hvac->component_size = 0.1;
        SPINNER_hvac_component_size->set_float_val(glui_hvac->component_size);
      }
      if(glui_hvac->filter_size < 0.1){
        glui_hvac->filter_size = 0.1;
        SPINNER_hvac_filter_size->set_float_val(glui_hvac->filter_size);
      }
      if(glui_hvac->node_size<1.0){
        glui_hvac->node_size = 1.0;
        SPINNER_hvac_node_size->set_float_val(glui_hvac->node_size);
      }
      if(glui_hvac->cell_node_size<1.0){
        glui_hvac->cell_node_size = 1.0;
        SPINNER_hvac_cell_node_size->set_float_val(glui_hvac->cell_node_size);
      }
      Glui2HVAC();
      break;
    case HVAC_SHOW_NETWORKS:
      if(hvac_show_networks==1){
        PANEL_hvac_network->enable();
        if(nhvacconnectinfo > 0){
          PANEL_hvac_connections->disable();
          hvac_show_connections = 0;
          CHECKBOX_hvac_show_connection->set_int_val(hvac_show_connections);
        }
      }
      else{
        PANEL_hvac_network->disable();
      }
      break;
    case HVAC_SHOW_CONNECTIONS:
      if(hvac_show_connections==1){
        PANEL_hvac_network->disable();
        if(PANEL_hvac_connections!=NULL)PANEL_hvac_connections->enable();
        hvac_show_networks = 0;
        if(CHECKBOX_hvac_show_network!=NULL)CHECKBOX_hvac_show_network->set_int_val(hvac_show_networks);
      }
      else{
        if(PANEL_hvac_connections != NULL)PANEL_hvac_connections->disable();
      }
      break;
    case HVAC_SHOWALL_NETWORK:
    case HVAC_HIDEALL_NETWORK:
      for(i=0;i<nhvacinfo;i++){
        hvacdata *hvaci;

        hvaci = hvacinfo + i;
        if(var==HVAC_SHOWALL_NETWORK)hvaci->display = 1;
        if(var==HVAC_HIDEALL_NETWORK)hvaci->display = 0;
        CHECKBOX_hvac_show_networks[i]->set_int_val(hvaci->display);
      }
      break;
    case HVAC_SHOWALL_CONNECTIONS:
    case HVAC_HIDEALL_CONNECTIONS:
      for (i = 0; i < nhvacconnectinfo; i++){
        hvacconnectdata *hi;

        hi = hvacconnectinfo + i;
        if(var==HVAC_SHOWALL_CONNECTIONS)hi->display = 1;
        if(var==HVAC_HIDEALL_CONNECTIONS)hi->display = 0;
        CHECKBOX_hvac_show_connections[i]->set_int_val(hi->display);
      }
      break;
    default:
      assert(FFALSE);
      break;
  }
  GLUTPOSTREDISPLAY;
}

#ifdef pp_DECIMATE
/* ------------------ AverageTerrainSize ------------------------ */

float AverageTerrainSize(void){
  int i;
  float total_distance, average_distance;
  int ntriangles = 0;
  int count, skip;

  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    meshi->decimated = 0;
  }
  for(i = 0; i < npatchinfo; i++){
    meshdata *meshi;
    patchdata *patchi;
    geomlistdata *geomlisti;

    patchi = patchinfo + i;
    if(patchi->loaded == 0 || patchi->display == 0 || patchi->blocknumber < 0)continue;
    meshi = meshinfo + patchi->blocknumber;
    if(meshi->decimated == 1)continue;
    if(patchi->geominfo == NULL || patchi->geominfo->display == 0 || patchi->geominfo->loaded == 0)continue;
    meshi->decimated = 1;
    geomlisti = patchi->geominfo->geomlistinfo - 1;
    ntriangles += geomlisti->ntriangles;
  }
  skip = MAX(ntriangles / 5000, 1);
  for(i = 0; i < nmeshes; i++){
    meshdata *meshi;

    meshi = meshinfo + i;
    meshi->decimated = 0;
  }
  count = 0;
  total_distance = 0.0;
  for(i = 0; i < npatchinfo; i++){
    meshdata *meshi;
    patchdata *patchi;
    geomlistdata *geomlisti;

    patchi = patchinfo + i;
    if(patchi->loaded == 0 || patchi->display == 0 || patchi->blocknumber < 0)continue;
    meshi = meshinfo + patchi->blocknumber;
    if(meshi->decimated == 1)continue;
    if(patchi->geominfo == NULL || patchi->geominfo->display == 0 || patchi->geominfo->loaded == 0)continue;
    meshi->decimated = 1;
    geomlisti = patchi->geominfo->geomlistinfo - 1;
    int j;

    for(j = 0; j < geomlisti->ntriangles; j += skip){
      vertdata *v1, *v2, *v3;
      float dist1, dist2, dist3, dx, dy, dz;

      v1 = geomlisti->triangles->verts[0];
      v2 = geomlisti->triangles->verts[1];
      v3 = geomlisti->triangles->verts[2];
      DDIST3(v1->xyz, v2->xyz, dist1);
      DDIST3(v1->xyz, v3->xyz, dist2);
      DDIST3(v2->xyz, v3->xyz, dist3);
      total_distance += (dist1+dist2+dist3);
      count+=3;
    }
  }
  if(count > 0){
    average_distance = total_distance / ( float )count;
  }
  else{
    average_distance = 0.0;
  }
  return average_distance;
}

/* ------------------ UpdateTerrainSizes ------------------------ */

void UpdateTerrainSizes(void){
  float domain_width, cell_width, pixel_width;
  int screen_width;
  float average_terrain_size;

  screen_width = glutGet(GLUT_SCREEN_WIDTH);                      // number of pixels
  domain_width = xbarORIG - xbar0ORIG;                            // meters
  cell_width = meshinfo->xplt_orig[1]-meshinfo->xplt_orig[0];
  pixel_width = domain_width / ( float )screen_width;

  char label[500], cval[500];

  Float2String(cval, pixel_width, 3, 1);
  sprintf(label, "pixel: %s m", cval);
  STATIC_terrain_pixel_size->set_name(label);

  Float2String(cval, cell_width, 3, 1);
  sprintf(label, "cell: %s m", cval);
  STATIC_terrain_cell_size->set_name(label);

  average_terrain_size = AverageTerrainSize();
  if(average_terrain_size > 0.0){
    Float2String(cval, average_terrain_size, 3, 1);
    sprintf(label, "geom: %s m", cval);
  }
  else{
    strcpy(label, "geom:");
  }
  STATIC_terrain_geom_size->set_name(label);
}
#endif

/* ------------------ GluiGeometrySetup ------------------------ */

extern "C" void GluiGeometrySetup(int main_window){
  int ibar,jbar,kbar;
  float *xplt_orig, *yplt_orig, *zplt_orig;
  char *surfacelabel;
  int i;

  ibar=current_mesh->ibar;
  jbar=current_mesh->jbar;
  kbar=current_mesh->kbar;
  xplt_orig=current_mesh->xplt_orig;
  yplt_orig=current_mesh->yplt_orig;
  zplt_orig=current_mesh->zplt_orig;

  if(glui_geometry!=NULL){
    glui_geometry->close();
    glui_geometry=NULL;
  }
  glui_geometry = GLUI_Master.create_glui("Geometry",0,0,0);
  if(showedit_dialog==0)glui_geometry->hide();

  if(nhvacinfo > 0){
    NewMemory(( void ** )&glui_hvac, sizeof(hvacdata));
    memcpy(glui_hvac, hvacinfo, sizeof(hvacdata));
    ROLLOUT_hvac = glui_geometry->add_rollout("HVAC", false, HVAC_ROLLOUT, GeomRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_hvac, glui_geometry);
    ADDPROCINFO(geomprocinfo, ngeomprocinfo, ROLLOUT_hvac, HVAC_ROLLOUT, glui_geometry);

    NewMemory((void **)&CHECKBOX_hvac_show_networks, nhvacinfo*sizeof(GLUI_Checkbox *));
    PANEL_hvac_options = glui_geometry->add_panel_to_panel(ROLLOUT_hvac, "", false);
    PANEL_hvac_options->set_alignment(GLUI_ALIGN_LEFT);
    CHECKBOX_hvac_metro_view = glui_geometry->add_checkbox_to_panel(PANEL_hvac_options, "metro view", &hvac_metro_view, HVAC_METRO_VIEW, HvacCB);
    CHECKBOX_hvac_cell_view = glui_geometry->add_checkbox_to_panel(PANEL_hvac_options, "show cells", &hvac_cell_view, HVAC_CELL_VIEW, HvacCB);
    if(nhvacconnectinfo > 0){
      CHECKBOX_hvac_show_network = glui_geometry->add_checkbox_to_panel(PANEL_hvac_options, "Show by network", &hvac_show_networks, HVAC_SHOW_NETWORKS, HvacCB);
      CHECKBOX_hvac_show_connection = glui_geometry->add_checkbox_to_panel(PANEL_hvac_options, "Show by connection", &hvac_show_connections, HVAC_SHOW_CONNECTIONS, HvacCB);
    }
    if(nhvacconnectinfo > 0){
      PANEL_hvac_group1 = glui_geometry->add_panel_to_panel(ROLLOUT_hvac, "", false);
      PANEL_hvac_network = glui_geometry->add_panel_to_panel(PANEL_hvac_group1, "networks");
    }
    else{
      hvac_show_networks = 1;
      PANEL_hvac_network = glui_geometry->add_panel_to_panel(ROLLOUT_hvac, "networks");
    }
    for (i = 0; i < nhvacinfo; i++){
      hvacdata* hvaci;

      hvaci = hvacinfo + i;
      CHECKBOX_hvac_show_networks[i] = glui_geometry->add_checkbox_to_panel(PANEL_hvac_network, hvaci->network_name, &hvaci->display);
    }
    if(nhvacinfo > 1){
      glui_geometry->add_button_to_panel(PANEL_hvac_network, "show all", HVAC_SHOWALL_NETWORK, HvacCB);
      glui_geometry->add_button_to_panel(PANEL_hvac_network, "hide all", HVAC_HIDEALL_NETWORK, HvacCB);
      //      glui_geometry->add_checkbox_to_panel(ROLLOUT_hvac, "copy settings to all networks", &hvac_copy_all, HVAC_COPY_ALL, HvacCB);
    }

    if(nhvacconnectinfo>0){
      NewMemory((void **)&CHECKBOX_hvac_show_connections, nhvacconnectinfo*sizeof(GLUI_Checkbox *));
      glui_geometry->add_column_to_panel(PANEL_hvac_group1, false);
      PANEL_hvac_connections = glui_geometry->add_panel_to_panel(PANEL_hvac_group1, "connections");
      for (i = 0; i < nhvacconnectinfo; i++){
        hvacconnectdata *hi;
        char label[100];

        hi = hvacconnectinfo + i;
        snprintf(label, sizeof(label), "%i", hi->index);
        CHECKBOX_hvac_show_connections[i] = glui_geometry->add_checkbox_to_panel(PANEL_hvac_connections, label, &hi->display);
      }
      if(nhvacconnectinfo > 1){
        glui_geometry->add_button_to_panel(PANEL_hvac_connections, "show all", HVAC_SHOWALL_CONNECTIONS, HvacCB);
        glui_geometry->add_button_to_panel(PANEL_hvac_connections, "hide all", HVAC_HIDEALL_CONNECTIONS, HvacCB);
      }
    }

    LIST_hvac_network_ductnode_index = glui_geometry->add_listbox_to_panel(ROLLOUT_hvac, "set duct/node properties for:", &hvac_network_ductnode_index, HVAC_DUCTNODE_NETWORK, HvacCB);
    LIST_hvac_network_ductnode_index->add_item(-1, "all networks");
    for(i = 0; i<nhvacinfo; i++){
      hvacdata *hvaci;

      hvaci = hvacinfo + i;
      LIST_hvac_network_ductnode_index->add_item(i, hvaci->network_name);
    }

    PANEL_hvac_group2 = glui_geometry->add_panel_to_panel(ROLLOUT_hvac, "", false);
    PANEL_hvac_duct                = glui_geometry->add_panel_to_panel(PANEL_hvac_group2,       "duct properties");

    SPINNER_hvac_duct_color[0] = glui_geometry->add_spinner_to_panel(PANEL_hvac_duct, "red", GLUI_SPINNER_INT, glui_hvac->duct_color, HVAC_PROPS, HvacCB);
    SPINNER_hvac_duct_color[1] = glui_geometry->add_spinner_to_panel(PANEL_hvac_duct, "green", GLUI_SPINNER_INT, glui_hvac->duct_color + 1, HVAC_PROPS, HvacCB);
    SPINNER_hvac_duct_color[2] = glui_geometry->add_spinner_to_panel(PANEL_hvac_duct, "blue", GLUI_SPINNER_INT, glui_hvac->duct_color + 2, HVAC_PROPS, HvacCB);
    SPINNER_hvac_duct_width        = glui_geometry->add_spinner_to_panel(PANEL_hvac_duct,  "line width", GLUI_SPINNER_FLOAT, &glui_hvac->duct_width,       HVAC_PROPS, HvacCB);
    CHECKBOX_hvac_show_duct_labels = glui_geometry->add_checkbox_to_panel(PANEL_hvac_duct, "show duct IDs", &glui_hvac->show_duct_labels, HVAC_PROPS, HvacCB);
    PANEL_hvac_components          = glui_geometry->add_panel_to_panel(PANEL_hvac_duct,       "components");
    RADIO_hvac_show_component_labels = glui_geometry->add_radiogroup_to_panel(PANEL_hvac_components, &glui_hvac->show_component, HVAC_PROPS, HvacCB);
    glui_geometry->add_radiobutton_to_group(RADIO_hvac_show_component_labels, "text");
    glui_geometry->add_radiobutton_to_group(RADIO_hvac_show_component_labels, "symbol");
    glui_geometry->add_radiobutton_to_group(RADIO_hvac_show_component_labels, "hide");
    SPINNER_hvac_component_size = glui_geometry->add_spinner_to_panel(PANEL_hvac_components, "size", GLUI_SPINNER_FLOAT, &glui_hvac->component_size, HVAC_PROPS, HvacCB);

    if(hvacductvalsinfo!=NULL&&hvacductvalsinfo->n_duct_vars>0){
      LIST_hvacductvar_index = glui_geometry->add_listbox_to_panel(PANEL_hvac_duct, "quantity:", &hvacductvar_index, HVAC_DUCT_LIST, HvacCB);
      LIST_hvacductvar_index->add_item(-1, "");
      for(i = 0;i < hvacductvalsinfo->n_duct_vars;i++){
        hvacvaldata *hi;

        hi = hvacductvalsinfo->duct_vars + i;
        LIST_hvacductvar_index->add_item(i, hi->label.shortlabel);
      }
      glui_geometry->add_button_to_panel(PANEL_hvac_duct, _("Set duct bounds"), HVACDUCT_SET_BOUNDS, HvacCB);
      glui_geometry->add_button_to_panel(PANEL_hvac_duct, _("Set node bounds"), HVACNODE_SET_BOUNDS, HvacCB);
    }

    glui_geometry->add_column_to_panel(PANEL_hvac_group2, false);
    PANEL_hvac_node                = glui_geometry->add_panel_to_panel(PANEL_hvac_group2, "node properties");

    SPINNER_hvac_node_color[0] = glui_geometry->add_spinner_to_panel(PANEL_hvac_node, "red", GLUI_SPINNER_INT, glui_hvac->node_color, HVAC_PROPS, HvacCB);
    SPINNER_hvac_node_color[1] = glui_geometry->add_spinner_to_panel(PANEL_hvac_node, "green", GLUI_SPINNER_INT, glui_hvac->node_color + 1, HVAC_PROPS, HvacCB);
    SPINNER_hvac_node_color[2] = glui_geometry->add_spinner_to_panel(PANEL_hvac_node, "blue", GLUI_SPINNER_INT, glui_hvac->node_color + 2, HVAC_PROPS, HvacCB);
    SPINNER_hvac_node_size         = glui_geometry->add_spinner_to_panel(PANEL_hvac_node, "node size",      GLUI_SPINNER_FLOAT, &glui_hvac->node_size,      HVAC_PROPS, HvacCB);
    SPINNER_hvac_cell_node_size    = glui_geometry->add_spinner_to_panel(PANEL_hvac_node, "cell node size", GLUI_SPINNER_FLOAT, &glui_hvac->cell_node_size, HVAC_PROPS, HvacCB);
    CHECKBOX_hvac_show_node_labels = glui_geometry->add_checkbox_to_panel(PANEL_hvac_node, "show node IDs",            &glui_hvac->show_node_labels, HVAC_PROPS, HvacCB);
    PANEL_hvac_filter              = glui_geometry->add_panel_to_panel(PANEL_hvac_node,       "filter");
    RADIO_hvac_show_filters        = glui_geometry->add_radiogroup_to_panel(PANEL_hvac_filter, &glui_hvac->show_filters, HVAC_PROPS, HvacCB);
    glui_geometry->add_radiobutton_to_group(RADIO_hvac_show_filters, "text");
    glui_geometry->add_radiobutton_to_group(RADIO_hvac_show_filters, "symbol");
    glui_geometry->add_radiobutton_to_group(RADIO_hvac_show_filters, "hide");
    SPINNER_hvac_filter_size = glui_geometry->add_spinner_to_panel(PANEL_hvac_filter, "size", GLUI_SPINNER_FLOAT, &glui_hvac->filter_size, HVAC_PROPS, HvacCB);
    for(i = 0; i < 3; i++){
      SPINNER_hvac_duct_color[i]->set_int_limits(0, 255);
      SPINNER_hvac_node_color[i]->set_int_limits(0, 255);
    }
    if(hvacnodevalsinfo!=NULL&&hvacnodevalsinfo->n_node_vars>0){
      LIST_hvacnodevar_index = glui_geometry->add_listbox_to_panel(PANEL_hvac_node, "quantity:", &hvacnodevar_index, HVAC_NODE_LIST, HvacCB);
      LIST_hvacnodevar_index->add_item(-1, "");
      for(i = 0;i < hvacnodevalsinfo->n_node_vars;i++){
        hvacvaldata *hi;

        hi = hvacnodevalsinfo->node_vars + i;
        LIST_hvacnodevar_index->add_item(i, hi->label.shortlabel);
      }
      glui_geometry->add_button_to_panel(PANEL_hvac_node, _("Set duct bounds"), HVACDUCT_SET_BOUNDS, HvacCB);
      glui_geometry->add_button_to_panel(PANEL_hvac_node, _("Set node bounds"), HVACNODE_SET_BOUNDS, HvacCB);
    }
    HvacCB(HVAC_PROPS);
    HvacCB(HVAC_SHOW_NETWORKS);
    HvacCB(HVAC_SHOW_CONNECTIONS);
  }

  if(have_obsts == 1){
    ROLLOUT_structured = glui_geometry->add_rollout("Structured", false, STRUCTURED_ROLLOUT, GeomRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_structured, glui_geometry);
    ADDPROCINFO(geomprocinfo, ngeomprocinfo, ROLLOUT_structured, STRUCTURED_ROLLOUT, glui_geometry);

    if(structured_isopen==1)ROLLOUT_structured->open();
    PANEL_obj_select = glui_geometry->add_panel_to_panel(ROLLOUT_structured, "SURFs");

    PANEL_faces = glui_geometry->add_panel_to_panel(PANEL_obj_select, "", GLUI_PANEL_NONE);

    glui_geometry->add_column_to_panel(PANEL_faces, false);

    if(nsurfinfo>0){
      glui_geometry->add_statictext_to_panel(PANEL_faces, "");

      LIST_obst_surface[DOWN_X] = glui_geometry->add_listbox_to_panel(PANEL_faces, _("Left"), surface_indices+DOWN_X, UPDATE_LIST, ObjectCB);
      LIST_obst_surface[DOWN_X]->set_w(260);
      for(i = 0; i<nsurfinfo; i++){
        surfdata *surfi;

        surfi = surfinfo+sorted_surfidlist[i];
        if(surfi->used_by_obst!=1)continue;
        if(surfi->obst_surface==0)continue;
        surfacelabel = surfi->surfacelabel;
        LIST_obst_surface[DOWN_X]->add_item(i, surfacelabel);
      }

      LIST_obst_surface[UP_X] = glui_geometry->add_listbox_to_panel(PANEL_faces, _("Right"), surface_indices+UP_X, UPDATE_LIST, ObjectCB);
      LIST_obst_surface[UP_X]->set_w(260);
      for(i = 0; i<nsurfinfo; i++){
        surfdata *surfi;

        surfi = surfinfo+sorted_surfidlist[i];
        if(surfi->used_by_obst!=1)continue;
        if(surfi->obst_surface==0)continue;
        surfacelabel = surfi->surfacelabel;
        LIST_obst_surface[UP_X]->add_item(i, surfacelabel);
      }

      LIST_obst_surface[DOWN_Y] = glui_geometry->add_listbox_to_panel(PANEL_faces, _("Front"), surface_indices+DOWN_Y, UPDATE_LIST, ObjectCB);
      LIST_obst_surface[DOWN_Y]->set_w(260);
      for(i = 0; i<nsurfinfo; i++){
        surfdata *surfi;

        surfi = surfinfo+sorted_surfidlist[i];
        if(surfi->used_by_obst!=1)continue;
        if(surfi->obst_surface==0)continue;
        surfacelabel = surfi->surfacelabel;
        LIST_obst_surface[DOWN_Y]->add_item(i, surfacelabel);
      }

      LIST_obst_surface[UP_Y] = glui_geometry->add_listbox_to_panel(PANEL_faces, _("Back"), surface_indices+UP_Y, UPDATE_LIST, ObjectCB);
      LIST_obst_surface[UP_Y]->set_w(260);
      for(i = 0; i<nsurfinfo; i++){
        surfdata *surfi;

        surfi = surfinfo+sorted_surfidlist[i];
        if(surfi->used_by_obst!=1)continue;
        if(surfi->obst_surface==0)continue;
        surfacelabel = surfi->surfacelabel;
        LIST_obst_surface[UP_Y]->add_item(i, surfacelabel);
      }

      LIST_obst_surface[DOWN_Z] = glui_geometry->add_listbox_to_panel(PANEL_faces, _("Down"), surface_indices+DOWN_Z, UPDATE_LIST, ObjectCB);
      LIST_obst_surface[DOWN_Z]->set_w(260);
      for(i = 0; i<nsurfinfo; i++){
        surfdata *surfi;

        surfi = surfinfo+sorted_surfidlist[i];
        if(surfi->used_by_obst!=1)continue;
        if(surfi->obst_surface==0)continue;
        surfacelabel = surfi->surfacelabel;
        LIST_obst_surface[DOWN_Z]->add_item(i, surfacelabel);
      }

      LIST_obst_surface[UP_Z] = glui_geometry->add_listbox_to_panel(PANEL_faces, _("Up"), surface_indices+UP_Z, UPDATE_LIST, ObjectCB);
      LIST_obst_surface[UP_Z]->set_w(260);
      for(i = 0; i<nsurfinfo; i++){
        surfdata *surfi;

        surfi = surfinfo+sorted_surfidlist[i];
        if(surfi->used_by_obst!=1)continue;
        if(surfi->obst_surface==0)continue;
        surfacelabel = surfi->surfacelabel;
        LIST_obst_surface[UP_Z]->add_item(i, surfacelabel);
      }

      ObjectCB(RADIO_WALL);
      for(i = 0; i<6; i++){
        LIST_obst_surface[i]->disable();
      }
    }
    glui_geometry->add_column_to_panel(ROLLOUT_structured, false);

    PANEL_obj_stretch4 = glui_geometry->add_panel_to_panel(ROLLOUT_structured, "", GLUI_PANEL_NONE);

    {
      char meshlabel[255];

      strcpy(meshlabel, _("Mesh:"));
      strcat(meshlabel, meshinfo->label);
      STATIC_mesh_index = glui_geometry->add_statictext_to_panel(PANEL_obj_stretch4, meshlabel);

    }
    STATIC_blockage_index = glui_geometry->add_statictext_to_panel(PANEL_obj_stretch4, "&OBST number: ");
    STATIC_label          = glui_geometry->add_statictext_to_panel(PANEL_obj_stretch4, "&OBST label:");
    STATIC_id_label       = glui_geometry->add_statictext_to_panel(PANEL_obj_stretch4, "&OBST ID:");

    PANEL_obj_stretch2 = glui_geometry->add_panel_to_panel(ROLLOUT_structured, "Coordinates");

    if(blocklocation==BLOCKlocation_grid){
      blockage_snapped = 1;
    }
    else{
      blockage_snapped = 0;
    }
    blockage_as_input = 1-blockage_snapped;
    CHECKBOX_blockage = glui_geometry->add_checkbox_to_panel(PANEL_obj_stretch2, _("Dimensions snapped to grid"), &blockage_snapped,
                                                             BLOCKAGE_AS_INPUT, ObjectCB);
    PANEL_obj_stretch3 = glui_geometry->add_panel_to_panel(PANEL_obj_stretch2, "", GLUI_PANEL_NONE);
    EDIT_xmin = glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3, "x", GLUI_EDITTEXT_FLOAT, &glui_block_xmin, XMIN_SPIN, ObjectCB);
    EDIT_ymin = glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3, "y", GLUI_EDITTEXT_FLOAT, &glui_block_ymin, YMIN_SPIN, ObjectCB);
    EDIT_zmin = glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3, "z", GLUI_EDITTEXT_FLOAT, &glui_block_zmin, ZMIN_SPIN, ObjectCB);

    glui_geometry->add_column_to_panel(PANEL_obj_stretch3, false);
    EDIT_xmax = glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3, "", GLUI_EDITTEXT_FLOAT, &glui_block_xmax, XMAX_SPIN, ObjectCB);
    EDIT_ymax = glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3, "", GLUI_EDITTEXT_FLOAT, &glui_block_ymax, YMAX_SPIN, ObjectCB);
    EDIT_zmax = glui_geometry->add_edittext_to_panel(PANEL_obj_stretch3, "", GLUI_EDITTEXT_FLOAT, &glui_block_zmax, ZMAX_SPIN, ObjectCB);

    EDIT_xmin->disable();
    EDIT_ymin->disable();
    EDIT_zmin->disable();

    EDIT_xmax->disable();
    EDIT_ymax->disable();
    EDIT_zmax->disable();
    ObjectCB(BLOCKAGE_AS_INPUT);

    EDIT_xmin->set_float_limits(xplt_orig[0], xplt_orig[ibar], GLUI_LIMIT_CLAMP);
    EDIT_xmax->set_float_limits(xplt_orig[0], xplt_orig[ibar], GLUI_LIMIT_CLAMP);
    EDIT_ymin->set_float_limits(yplt_orig[0], yplt_orig[jbar], GLUI_LIMIT_CLAMP);
    EDIT_ymax->set_float_limits(yplt_orig[0], yplt_orig[jbar], GLUI_LIMIT_CLAMP);
    EDIT_zmin->set_float_limits(zplt_orig[0], zplt_orig[kbar], GLUI_LIMIT_CLAMP);
    EDIT_zmax->set_float_limits(zplt_orig[0], zplt_orig[kbar], GLUI_LIMIT_CLAMP);
  }

  if(ngeominfo>0){
    ROLLOUT_unstructured = glui_geometry->add_rollout("Immersed", false, UNSTRUCTURED_ROLLOUT, GeomRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_unstructured, glui_geometry);
    ADDPROCINFO(geomprocinfo, ngeomprocinfo, ROLLOUT_unstructured, UNSTRUCTURED_ROLLOUT, glui_geometry);
    if(unstructured_isopen==1)ROLLOUT_unstructured->open();

    for(i = 0; i<nmeshes; i++){
      meshdata *meshi;

      meshi = meshinfo+i;
      if(meshi->ncutcells>0){
        glui_geometry->add_checkbox_to_panel(ROLLOUT_unstructured, _("Show cutcells"), &show_cutcells);
        break;
      }
    }

    PANEL_geom_showhide = glui_geometry->add_panel_to_panel(ROLLOUT_unstructured, "", GLUI_PANEL_NONE);
    PANEL_group1 = glui_geometry->add_panel_to_panel(PANEL_geom_showhide, "", GLUI_PANEL_NONE);
    PANEL_face_cface = glui_geometry->add_panel_to_panel(PANEL_group1, "", GLUI_PANEL_NONE);
    PANEL_face_cface->set_alignment(GLUI_ALIGN_LEFT);
    PANEL_triangles = glui_geometry->add_panel_to_panel(PANEL_face_cface, "faces");
    PANEL_triangles->set_alignment(GLUI_ALIGN_LEFT);
    CHECKBOX_surface_solid = glui_geometry->add_checkbox_to_panel(PANEL_triangles,   "solid",   &show_faces_shaded,  VOL_SHOWHIDE, VolumeCB);
    CHECKBOX_surface_outline = glui_geometry->add_checkbox_to_panel(PANEL_triangles, "outline", &show_faces_outline, VOL_SHOWHIDE, VolumeCB);
    CHECKBOX_surface_points = glui_geometry->add_checkbox_to_panel(PANEL_triangles,  "points",  &show_geom_verts,    VOL_SHOWHIDE, VolumeCB);

    if(ncgeominfo>0){
      glui_geometry->add_column_to_panel(PANEL_face_cface, false);
      PANEL_cfaces = glui_geometry->add_panel_to_panel(PANEL_face_cface, "cfaces");
      PANEL_cfaces->set_alignment(GLUI_ALIGN_LEFT);
      CHECKBOX_cfaces = glui_geometry->add_checkbox_to_panel(PANEL_cfaces, "show", &glui_use_cfaces, VOL_USE_CFACES, VolumeCB);
      RADIO_cface_type = glui_geometry->add_radiogroup_to_panel(PANEL_cfaces, &geom_cface_type);
      glui_geometry->add_radiobutton_to_group(RADIO_cface_type, "triangles");
      glui_geometry->add_radiobutton_to_group(RADIO_cface_type, "polygons");
      VolumeCB(VOL_USE_CFACES);
      if(have_cface_normals==CFACE_NORMALS_YES){
        CHECKBOX_show_cface_normals = glui_geometry->add_checkbox_to_panel(PANEL_cfaces, "normal vectors", &show_cface_normals);
      }
    }

    PANEL_geom_offset_outline = glui_geometry->add_panel_to_panel(PANEL_group1, "offset outline/points");
    PANEL_geom_offset_outline->set_alignment(GLUI_ALIGN_LEFT);
    glui_geometry->add_spinner_to_panel(PANEL_geom_offset_outline, "normal",   GLUI_SPINNER_FLOAT, &geom_norm_offset);
    glui_geometry->add_spinner_to_panel(PANEL_geom_offset_outline, "vertical", GLUI_SPINNER_FLOAT, &geom_dz_offset);

    PANEL_boundingbox = glui_geometry->add_panel_to_panel(PANEL_group1, "show bounding box");
    PANEL_boundingbox->set_alignment(GLUI_ALIGN_LEFT);
    RADIO_show_geom_boundingbox = glui_geometry->add_radiogroup_to_panel(PANEL_boundingbox, &show_geom_boundingbox, GEOM_BOUNDING_BOX, VolumeCB);
    glui_geometry->add_radiobutton_to_group(RADIO_show_geom_boundingbox, "always");
    glui_geometry->add_radiobutton_to_group(RADIO_show_geom_boundingbox, "when mouse is pressed");
    glui_geometry->add_radiobutton_to_group(RADIO_show_geom_boundingbox, "never");

    PANEL_geom_transparency = glui_geometry->add_panel_to_panel(PANEL_group1, "transparency");
    PANEL_geom_transparency->set_alignment(GLUI_ALIGN_LEFT);
    CHECKBOX_geom_force_transparent = glui_geometry->add_checkbox_to_panel(PANEL_geom_transparency, "force", &geom_force_transparent);
    SPINNER_geom_transparency = glui_geometry->add_spinner_to_panel(PANEL_geom_transparency, "level", GLUI_SPINNER_FLOAT, &geom_transparency);
    SPINNER_geom_transparency->set_float_limits(0.0, 1.0);

    PANEL_normals = glui_geometry->add_panel_to_panel(PANEL_group1, "normals");
    PANEL_normals->set_alignment(GLUI_ALIGN_LEFT);
    GLUI_Panel *PANEL_show_smooth = glui_geometry->add_panel_to_panel(PANEL_normals, "", GLUI_PANEL_NONE);
    CHECKBOX_show_geom_normal = glui_geometry->add_checkbox_to_panel(PANEL_show_smooth, "show", &show_geom_normal);
    glui_geometry->add_column_to_panel(PANEL_show_smooth, false);
    CHECKBOX_smooth_geom_normal = glui_geometry->add_checkbox_to_panel(PANEL_show_smooth, "smooth", &smooth_geom_normal);
    SPINNER_geom_ivecfactor = glui_geometry->add_spinner_to_panel(PANEL_normals, "length", GLUI_SPINNER_INT, &geom_ivecfactor, GEOM_IVECFACTOR, VolumeCB);
    SPINNER_geom_ivecfactor->set_int_limits(0, 200);

    PANEL_outlinecolor = glui_geometry->add_panel_to_panel(PANEL_group1, "outline color when grid is shown");
    SPINNER_outlinecolor_red   = glui_geometry->add_spinner_to_panel(PANEL_outlinecolor, "red",   GLUI_SPINNER_INT, glui_outlinecolor  , GEOM_OUTLINECOLOR, VolumeCB);
    SPINNER_outlinecolor_green = glui_geometry->add_spinner_to_panel(PANEL_outlinecolor, "green", GLUI_SPINNER_INT, glui_outlinecolor+1, GEOM_OUTLINECOLOR, VolumeCB);
    SPINNER_outlinecolor_blue  = glui_geometry->add_spinner_to_panel(PANEL_outlinecolor, "blue",  GLUI_SPINNER_INT, glui_outlinecolor+2, GEOM_OUTLINECOLOR, VolumeCB);
    SPINNER_outlinecolor_red->set_int_limits(0, 255);
    SPINNER_outlinecolor_green->set_int_limits(0, 255);
    SPINNER_outlinecolor_blue->set_int_limits(0, 255);

    glui_geometry->add_column_to_panel(PANEL_group1, false);

    UpdateGeomAreas();

    ROLLOUT_geom_properties = glui_geometry->add_rollout("Immersed diagnostics",false, IMMERSED_DIAGNOSTICS, GeomRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_geom_properties, glui_geometry);
    ADDPROCINFO(geomprocinfo, ngeomprocinfo, ROLLOUT_geom_properties, IMMERSED_DIAGNOSTICS, glui_geometry);
    PANEL_properties2 = glui_geometry->add_panel_to_panel(ROLLOUT_geom_properties,"",GLUI_PANEL_NONE);

    RADIO_select_geom = glui_geometry->add_radiogroup_to_panel(PANEL_properties2, &select_geom, SELECT_GEOM,VolumeCB);
    glui_geometry->add_radiobutton_to_group(RADIO_select_geom, "none");
    glui_geometry->add_radiobutton_to_group(RADIO_select_geom, "vertex 1");
    glui_geometry->add_radiobutton_to_group(RADIO_select_geom, "vertex 2");
    glui_geometry->add_radiobutton_to_group(RADIO_select_geom, "triangle");
    glui_geometry->add_radiobutton_to_group(RADIO_select_geom, "surf");
    glui_geometry->add_column_to_panel(PANEL_properties2, false);

    PANEL_properties_vertex = glui_geometry->add_panel_to_panel(PANEL_properties2, "vertex");
    STATIC_vertx1 = glui_geometry->add_statictext_to_panel(PANEL_properties_vertex, "x1:");
    STATIC_verty1 = glui_geometry->add_statictext_to_panel(PANEL_properties_vertex, "y1:");
    STATIC_vertz1 = glui_geometry->add_statictext_to_panel(PANEL_properties_vertex, "z1:");
    STATIC_dist   = glui_geometry->add_statictext_to_panel(PANEL_properties_vertex, "dist:");

    glui_geometry->add_column_to_panel(PANEL_properties_vertex,false);

    STATIC_vertx2 = glui_geometry->add_statictext_to_panel(PANEL_properties_vertex, "x2:");
    STATIC_verty2 = glui_geometry->add_statictext_to_panel(PANEL_properties_vertex, "y2:");
    STATIC_vertz2 = glui_geometry->add_statictext_to_panel(PANEL_properties_vertex, "z2:");
    UpdateVertexInfo(NULL, NULL);

    PANEL_properties_triangle = glui_geometry->add_panel_to_panel(PANEL_properties2, "triangle");
    STATIC_tri_area = glui_geometry->add_statictext_to_panel(PANEL_properties_triangle, "area:");

    glui_geometry->add_column_to_panel(PANEL_properties2, false);

    PANEL_properties_surf = glui_geometry->add_panel_to_panel(PANEL_properties2, "SURF");
    LIST_geom_surface = glui_geometry->add_listbox_to_panel(PANEL_properties_surf, _("id:"), &geom_surf_index, SURF_GET, VolumeCB);
    {
      int ii;

      ii = 0;
      for(i = 0; i<nsurfinfo; i++){
        surfdata *surfi;

        surfi = surfinfo+sorted_surfidlist[i];
        if(surfi->used_by_geom==1){
          char label[100];

          surfi->in_geom_list = ii;
//          sprintf(label, "%s/%f m2", surfi->surfacelabel, surfi->geom_area);
          snprintf(label, sizeof(label), "%s/%f m2", surfi->surfacelabel, surfi->geom_area);
          LIST_geom_surface->add_item(ii, label);
          ii++;
        }
      }
    }
    PANEL_surf_coloraxis = glui_geometry->add_panel_to_panel(PANEL_properties_surf, "", GLUI_PANEL_NONE);
    PANEL_surf_color = glui_geometry->add_panel_to_panel(PANEL_surf_coloraxis, "color");
    glui_geometry->add_checkbox_to_panel(PANEL_surf_color, "use", &use_surf_color);
    SPINNER_surf_rgb[0] = glui_geometry->add_spinner_to_panel(PANEL_surf_color, "red",   GLUI_SPINNER_INT, glui_surf_rgb+0, SURF_SET, VolumeCB);
    SPINNER_surf_rgb[1] = glui_geometry->add_spinner_to_panel(PANEL_surf_color, "green", GLUI_SPINNER_INT, glui_surf_rgb+1, SURF_SET, VolumeCB);
    SPINNER_surf_rgb[2] = glui_geometry->add_spinner_to_panel(PANEL_surf_color, "blue",  GLUI_SPINNER_INT, glui_surf_rgb+2, SURF_SET, VolumeCB);

    glui_geometry->add_column_to_panel(PANEL_surf_coloraxis, false);
    PANEL_surf_axis = glui_geometry->add_panel_to_panel(PANEL_surf_coloraxis, "axis");
    glui_surf_axis[0] = 0.0;
    glui_surf_axis[1] = 0.0;
    glui_surf_axis[2] = 0.0;
    glui_geometry->add_checkbox_to_panel(PANEL_surf_axis, "show", &show_surf_axis);
    SPINNER_surf_axis[0] = glui_geometry->add_spinner_to_panel(PANEL_surf_axis, "x", GLUI_SPINNER_FLOAT, glui_surf_axis+0, SURF_SET, VolumeCB);
    SPINNER_surf_axis[1] = glui_geometry->add_spinner_to_panel(PANEL_surf_axis, "y", GLUI_SPINNER_FLOAT, glui_surf_axis+1, SURF_SET, VolumeCB);
    SPINNER_surf_axis[2] = glui_geometry->add_spinner_to_panel(PANEL_surf_axis, "z", GLUI_SPINNER_FLOAT, glui_surf_axis+2, SURF_SET, VolumeCB);
    glui_geometry->add_spinner_to_panel(PANEL_surf_axis, "length", GLUI_SPINNER_FLOAT, &glui_surf_axis_length);
    glui_geometry->add_spinner_to_panel(PANEL_surf_axis, "width",  GLUI_SPINNER_FLOAT, &glui_surf_axis_width);

    VolumeCB(SURF_GET);

    ROLLOUT_geom_rgbs = glui_geometry->add_rollout_to_panel(ROLLOUT_geom_properties, "Selection colors",false);

    PANEL_vertex1_rgb = glui_geometry->add_panel_to_panel(ROLLOUT_geom_rgbs, "vertex 1");
    SPINNER_geom_vertex1_rgb[0] = glui_geometry->add_spinner_to_panel(PANEL_vertex1_rgb, "red",   GLUI_SPINNER_INT, geom_vertex1_rgb+0);
    SPINNER_geom_vertex1_rgb[1] = glui_geometry->add_spinner_to_panel(PANEL_vertex1_rgb, "green", GLUI_SPINNER_INT, geom_vertex1_rgb+1);
    SPINNER_geom_vertex1_rgb[2] = glui_geometry->add_spinner_to_panel(PANEL_vertex1_rgb, "blue",  GLUI_SPINNER_INT, geom_vertex1_rgb+2);
    glui_geometry->add_column_to_panel(ROLLOUT_geom_rgbs, false);

    PANEL_vertex2_rgb = glui_geometry->add_panel_to_panel(ROLLOUT_geom_rgbs, "vertex 2");
    SPINNER_geom_vertex2_rgb[0] = glui_geometry->add_spinner_to_panel(PANEL_vertex2_rgb, "red",   GLUI_SPINNER_INT, geom_vertex2_rgb+0);
    SPINNER_geom_vertex2_rgb[1] = glui_geometry->add_spinner_to_panel(PANEL_vertex2_rgb, "green", GLUI_SPINNER_INT, geom_vertex2_rgb+1);
    SPINNER_geom_vertex2_rgb[2] = glui_geometry->add_spinner_to_panel(PANEL_vertex2_rgb, "blue",  GLUI_SPINNER_INT, geom_vertex2_rgb+2);
    glui_geometry->add_column_to_panel(ROLLOUT_geom_rgbs, false);

    PANEL_triangle_rgb = glui_geometry->add_panel_to_panel(ROLLOUT_geom_rgbs, "triangle/surf");
    SPINNER_geom_triangle_rgb[0] = glui_geometry->add_spinner_to_panel(PANEL_triangle_rgb, "red",   GLUI_SPINNER_INT, geom_triangle_rgb+0);
    SPINNER_geom_triangle_rgb[1] = glui_geometry->add_spinner_to_panel(PANEL_triangle_rgb, "green", GLUI_SPINNER_INT, geom_triangle_rgb+1);
    SPINNER_geom_triangle_rgb[2] = glui_geometry->add_spinner_to_panel(PANEL_triangle_rgb, "blue",  GLUI_SPINNER_INT, geom_triangle_rgb+2);

    for(i = 0; i<3; i++){
      SPINNER_geom_vertex1_rgb[i]->set_int_limits(0, 255);
      SPINNER_geom_vertex2_rgb[i]->set_int_limits(0, 255);
      SPINNER_geom_triangle_rgb[i]->set_int_limits(0, 255);
    }

    PANEL_geom_show = glui_geometry->add_panel_to_panel(PANEL_group1, "show");
    PANEL_geom_show->set_alignment(GLUI_ALIGN_LEFT);
    if(terrain_nindices>0){
      CHECKBOX_showonly_top = glui_geometry->add_checkbox_to_panel(PANEL_geom_show, "only top surface", &terrain_showonly_top, SHOWONLY_TOP, VolumeCB);
    }
    CHECKBOX_showgeom_inside_domain = glui_geometry->add_checkbox_to_panel(PANEL_geom_show, "inside FDS domain", &showgeom_inside_domain, GEOM_FDS_DOMAIN, VolumeCB);
    CHECKBOX_showgeom_outside_domain = glui_geometry->add_checkbox_to_panel(PANEL_geom_show, "outside FDS domain", &showgeom_outside_domain, GEOM_FDS_DOMAIN, VolumeCB);
    glui_geometry->add_checkbox_to_panel(PANEL_geom_show, "geometry and boundary files", &glui_show_geom_bndf, UPDATE_GEOM, VolumeCB);

    PANEL_geomtest2 = glui_geometry->add_panel_to_panel(PANEL_group1, "settings");
    PANEL_geomtest2->set_alignment(GLUI_ALIGN_LEFT);

    glui_geometry->add_spinner_to_panel(PANEL_geomtest2, "line width", GLUI_SPINNER_FLOAT, &geom_linewidth);
    glui_geometry->add_spinner_to_panel(PANEL_geomtest2, "point size", GLUI_SPINNER_FLOAT, &geom_pointsize);

    SPINNER_geom_vert_exag = glui_geometry->add_spinner_to_panel(PANEL_geomtest2, "vertical exaggeration", GLUI_SPINNER_FLOAT, &geom_vert_exag, GEOM_VERT_EXAG, VolumeCB);
    SPINNER_geom_vert_exag->set_float_limits(0.1, 10.0);

    if(nterrain_textures>0){
      NewMemory((void **)&CHECKBOX_terrain_texture_show, sizeof(GLUI_Checkbox *)*nterrain_textures);
      PANEL_terrain_images = glui_geometry->add_panel_to_panel(PANEL_group1, "terrain images");
      for(i = 0; i<nterrain_textures; i++){
        texturedata *texti;

        texti = terrain_textures+i;
        if(texti->loaded==1){
          CHECKBOX_terrain_texture_show[i] = glui_geometry->add_checkbox_to_panel(PANEL_terrain_images, texti->file, &(texti->display), i, TerrainTextureCB);
        }
        else{
          CHECKBOX_terrain_texture_show[i] = NULL;
        }
      }
    }
#ifdef pp_DECIMATE
    if(nterraininfo > 0){
      terrain_decimate_delta     = MAX((xbarFDS - xbar0FDS)/screenWidth, (ybarFDS - ybar0FDS)/screenHeight);
      terrain_decimate_delta_min = terrain_decimate_delta/4.0;
      PANEL_terrain_decimate = glui_geometry->add_panel_to_panel(PANEL_group1, "Decimate terrain geometry");
      SPINNER_terrain_deimate_delta = glui_geometry->add_spinner_to_panel(PANEL_terrain_decimate, "delta", GLUI_SPINNER_FLOAT, &terrain_decimate_delta, GEOM_DECIMATE_DELTA, VolumeCB);
      CHECKBOX_use_decimate_geom = glui_geometry->add_checkbox_to_panel(PANEL_terrain_decimate, "use decimated geometry", &use_decimate_geom);
      BUTTON_reset_zbounds = glui_geometry->add_button_to_panel(PANEL_terrain_decimate, _("Decimate"), GEOM_DECIMATE, VolumeCB);
      PANEL_terrain_decimate_sizes = glui_geometry->add_panel_to_panel(PANEL_terrain_decimate, "Sizes (approximate)");
      STATIC_terrain_cell_size = glui_geometry->add_statictext_to_panel(PANEL_terrain_decimate_sizes, "cell:");
      STATIC_terrain_pixel_size = glui_geometry->add_statictext_to_panel(PANEL_terrain_decimate_sizes,"pixel:");
      STATIC_terrain_geom_size  = glui_geometry->add_statictext_to_panel(PANEL_terrain_decimate_sizes, "geom:");
      UpdateTerrainSizes();
    }
#endif

    PANEL_elevation_color = glui_geometry->add_panel_to_panel(PANEL_group1, "color by elevation");
    PANEL_elevation_color->set_alignment(GLUI_ALIGN_LEFT);
    CHECKBOX_show_texture_1dimage = glui_geometry->add_checkbox_to_panel(PANEL_elevation_color, "show elevation colors", &show_texture_1dimage, SHOW_TEXTURE_1D_IMAGE, VolumeCB);
    GetGeomZBounds(&terrain_zmin, &terrain_zmax);
    terrain_zlevel = (terrain_zmin+terrain_zmax)/2.0;
    CHECKBOX_show_zlevel = glui_geometry->add_checkbox_to_panel(PANEL_elevation_color, "hilight elevation", &show_zlevel, SHOW_ZLEVEL, VolumeCB);

    SPINNER_geom_zmin = glui_geometry->add_spinner_to_panel(PANEL_elevation_color, "zmin", GLUI_SPINNER_FLOAT, &terrain_zmin, TERRAIN_ZMIN, VolumeCB);
    SPINNER_geom_zmin->set_float_limits(zbar0ORIG, zbarORIG);

    SPINNER_geom_zmax = glui_geometry->add_spinner_to_panel(PANEL_elevation_color, "zmax", GLUI_SPINNER_FLOAT, &terrain_zmax, TERRAIN_ZMAX, VolumeCB);
    SPINNER_geom_zmax->set_float_limits(zbar0ORIG, zbarORIG);

    SPINNER_geom_zlevel = glui_geometry->add_spinner_to_panel(PANEL_elevation_color, "elevation", GLUI_SPINNER_FLOAT, &terrain_zlevel, TERRAIN_ZLEVEL, VolumeCB);
    SPINNER_geom_zlevel->set_float_limits(zbar0ORIG, zbarORIG);

    VolumeCB(GEOM_VERT_EXAG);
    BUTTON_reset_zbounds = glui_geometry->add_button_to_panel(PANEL_elevation_color, _("Reset"), RESET_ZBOUNDS, VolumeCB);
  }

  if(nterraininfo>0&&ngeominfo==0){
    ROLLOUT_terrain = glui_geometry->add_rollout("Terrain", false, TERRAIN_ROLLOUT, GeomRolloutCB);
    INSERT_ROLLOUT(ROLLOUT_terrain, glui_geometry);
    ADDPROCINFO(geomprocinfo, ngeomprocinfo, ROLLOUT_terrain, TERRAIN_ROLLOUT, glui_geometry);

    CHECKBOX_terrain_top_surface = glui_geometry->add_checkbox_to_panel(ROLLOUT_terrain, "Show only top surface",
      &terrain_showonly_top, TERRAIN_TOP_ONLY, TerrainCB);
    RADIO_terrain_type = glui_geometry->add_radiogroup_to_panel(ROLLOUT_terrain, &visTerrainType, TERRAIN_TYPE, TerrainCB);
    glui_geometry->add_radiobutton_to_group(RADIO_terrain_type, "3D surface");
    glui_geometry->add_radiobutton_to_group(RADIO_terrain_type, "Image");
    glui_geometry->add_radiobutton_to_group(RADIO_terrain_type, "Hidden");
    SPINNER_terrain_skip = glui_geometry->add_spinner_to_panel(ROLLOUT_terrain, "terrain skip", GLUI_SPINNER_INT, &terrain_skip);
    SPINNER_terrain_skip->set_int_limits(1, 10);
#define SLICE_SKIP 124
    SPINNER_slice_skip2 = glui_geometry->add_spinner_to_panel(ROLLOUT_terrain, "slice data skip", GLUI_SPINNER_INT, &slice_skip, SLICE_SKIP, SliceBoundCB);
    SliceBoundCB(SLICE_SKIP);
  }

  PANEL_geom_close = glui_geometry->add_panel("", GLUI_PANEL_NONE);

  glui_geometry->add_button_to_panel(PANEL_geom_close, _("Save settings"), SAVE_SETTINGS_GEOM, BlockeditDlgCB);

  glui_geometry->add_column_to_panel(PANEL_geom_close, false);

  BUTTON_blockage_1=glui_geometry->add_button_to_panel(PANEL_geom_close, _("Close"),CLOSE_WINDOW, BlockeditDlgCB);
#ifdef pp_CLOSEOFF
  BUTTON_blockage_1->disable();
#endif

  glui_geometry->set_main_gfx_window( main_window );
}

/* ------------------ TerrainCB ------------------------ */

extern "C" void TerrainCB(int var){
  switch (var){
    case TERRAIN_TYPE:
      GeometryMenu(17+visTerrainType);
      break;
    case TERRAIN_TOP_ONLY:
      terrain_showonly_top = 1 - terrain_showonly_top;
      GeometryMenu(17+TERRAIN_TOP);
      UpdateShowOnlyTop();
      break;
    default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ GetGeomZBounds ------------------------ */

void GetGeomZBounds(float *zmin, float *zmax){
  int i;
  geomlistdata *terrain;
  int first = 1;

  if(geominfo->geomlistinfo == NULL)return;
  terrain = geominfo->geomlistinfo - 1;

  for(i = 0; i < terrain->nverts; i++){
    vertdata *verti;
    float zval;

    verti = terrain->verts + i;
    zval = terrain->zORIG[i];
    if(terrain_showonly_top == 0 || verti->vert_norm[2] > 0.0){
      if(first == 1){
        *zmin = zval;
        *zmax = zval;
        first = 0;
      }
      else{
        *zmin = MIN(*zmin, zval);
        *zmax = MAX(*zmax, zval);
      }
    }
  }
}

/* ------------------ VolumeCB ------------------------ */

extern "C" void VolumeCB(int var){
  int i;
  switch(var){
  case SURF_GET:
    for(i = 0; i<nsurfinfo; i++){
      surfdata *surfi;

      surfi = surfinfo+sorted_surfidlist[i];
      if(surfi->in_geom_list==geom_surf_index){
        int *rgb_local;
        float *axis;

        rgb_local = surfi->geom_surf_color;
        glui_surf_rgb[0] = CLAMP(rgb_local[0],0,255);
        glui_surf_rgb[1] = CLAMP(rgb_local[1],0,255);
        glui_surf_rgb[2] = CLAMP(rgb_local[2],0,255);
        SPINNER_surf_rgb[0]->set_int_val(glui_surf_rgb[0]);
        SPINNER_surf_rgb[1]->set_int_val(glui_surf_rgb[1]);
        SPINNER_surf_rgb[2]->set_int_val(glui_surf_rgb[2]);

        axis = surfi->axis;
        glui_surf_axis[0] = axis[0];
        glui_surf_axis[1] = axis[1];
        glui_surf_axis[2] = axis[2];
        SPINNER_surf_axis[0]->set_int_val(glui_surf_axis[0]);
        SPINNER_surf_axis[1]->set_int_val(glui_surf_axis[1]);
        SPINNER_surf_axis[2]->set_int_val(glui_surf_axis[2]);
        break;
      }
    }
    break;
  case SURF_SET:
    for(i = 0; i<nsurfinfo; i++){
      surfdata *surfi;

      surfi = surfinfo+sorted_surfidlist[i];
      if(surfi->in_geom_list==geom_surf_index){
        int *rgb_local;
        float *axis;

        rgb_local = surfi->geom_surf_color;
        rgb_local[0] = glui_surf_rgb[0];
        rgb_local[1] = glui_surf_rgb[1];
        rgb_local[2] = glui_surf_rgb[2];

        axis = surfi->axis;
        axis[0] = glui_surf_axis[0];
        axis[1] = glui_surf_axis[1];
        axis[2] = glui_surf_axis[2];
        break;
      }
    }
    break;
  case SELECT_GEOM:
    switch(select_geom){
    case GEOM_PROP_NONE:
      selected_geom_vertex1 = -1;
      selected_geom_vertex2 = -1;
      selected_geom_triangle = -1;
      break;
    case GEOM_PROP_TRIANGLE:
    case GEOM_PROP_SURF:
      selected_geom_vertex1 = -1;
      selected_geom_vertex2 = -1;
      break;
    case GEOM_PROP_VERTEX1:
    case GEOM_PROP_VERTEX2:
      selected_geom_triangle = -1;
    default:
      assert(FFALSE);
      break;
    }
    break;
  case UPDATE_GEOM:
    show_geom_bndf = glui_show_geom_bndf;
    update_times = 1;
    break;
  case GEOM_VERT_EXAG:
    UpdateGeomNormals();
    break;
  case GEOM_FDS_DOMAIN:
  case SHOWONLY_TOP:
    updatemenu = 1;
    break;
  case SHOW_ZLEVEL:
    if(show_texture_1dimage==0&&show_zlevel==1){
      show_texture_1dimage = 1;
      CHECKBOX_show_texture_1dimage->set_int_val(show_texture_1dimage);
      VolumeCB(SHOW_TEXTURE_1D_IMAGE);
    }
  case TERRAIN_ZLEVEL:
    UpdateChopColors();
  break;
  case RESET_ZBOUNDS:
    GetGeomZBounds(&terrain_zmin, &terrain_zmax);
    terrain_zlevel = (terrain_zmin+terrain_zmax)/2.0;
    SPINNER_geom_zlevel->set_float_val(terrain_zlevel);
    SPINNER_geom_zmin->set_float_val(terrain_zmin);
    SPINNER_geom_zmax->set_float_val(terrain_zmax);
    SPINNER_geom_zlevel->set_float_limits(terrain_zmin, terrain_zmax);
  case TERRAIN_ZMIN:
  case TERRAIN_ZMAX:
    if(ABS(terrain_zmin - terrain_zmax) < 0.01){
      terrain_zmax = terrain_zmin + .01;
      SPINNER_geom_zmax->set_float_val(terrain_zmax);
    }
    SPINNER_geom_zlevel->set_float_limits(terrain_zmin, terrain_zmax);
    UpdateChopColors();
  case SHOW_TEXTURE_1D_IMAGE:
    if(show_texture_1dimage == 1&&nterrain_textures>0){
      for(i=0; i<nterrain_textures; i++){
        texturedata *texti;

        texti = terrain_textures+i;
        if(texti->loaded==1){
          texti->display = 0;
          UpdateTerrainTexture(i);
        }
      }
      updatemenu = 1;
    }
    break;
  case GEOM_OUTLINECOLOR:
    break;
#ifdef pp_DECIMATE
  case GEOM_DECIMATE:
    UpdateTerrainSizes();
    DecimateAllTerrains();
    break;
  case GEOM_DECIMATE_DELTA:
    if(terrain_decimate_delta < terrain_decimate_delta_min){
      terrain_decimate_delta = terrain_decimate_delta_min;
      SPINNER_terrain_deimate_delta->set_float_val(terrain_decimate_delta);
    }
    break;
#endif
  case GEOM_IVECFACTOR:
    geom_vecfactor = (float)geom_ivecfactor/1000.0;
    break;
    // update_volbox_controls=1;
    // face_vis
    // face_vis_old
  case VOL_SHOWHIDE:
    terrain_show_geometry_outline = show_faces_outline;
    terrain_show_geometry_points  = show_geom_verts;
    terrain_show_geometry_surface = show_faces_shaded;
    updatemenu=1;
    break;
  case VOL_USE_CFACES:
    VolumeCB(VOL_SHOWHIDE);
    Keyboard('q',FROM_GEOM_DIALOG);
    break;
  case GEOM_BOUNDING_BOX:
    updatemenu=1;
    break;
  default:
    assert(FFALSE);
    break;
  }
}

/* ------------------ HideGluiHVAC ------------------------ */

extern "C" void HideGluiHVAC(void){
  showhvac_dialog = 0;
  CloseRollouts(glui_geometry);
}

/* ------------------ ShowGluiHVAC ------------------------ */

extern "C" void ShowGluiHVAC(void){
  if(glui_geometry!=NULL && ROLLOUT_hvac!=NULL){
    showhvac_dialog=1;
    glui_geometry->show();
    ROLLOUT_hvac->open();
  }
}

/* ------------------ HideGluiGeometry ------------------------ */

extern "C" void HideGluiGeometry(void){
  blockageSelect=0;
  CloseRollouts(glui_geometry);
  showedit_dialog=0;
  editwindow_status=CLOSE_WINDOW;
}

/* ------------------ ShowGluiGeometry ------------------------ */

extern "C" void ShowGluiGeometry(void){
  showedit_dialog=1;
  blockageSelect=1;
  UpdateBlockVals(NOT_SELECT_BLOCKS);
  if(glui_geometry!=NULL){
    glui_geometry->show();
    if(ROLLOUT_unstructured!=NULL&&ROLLOUT_structured==NULL){
      ROLLOUT_unstructured->open();
    }
    else if(ROLLOUT_structured!=NULL&&ROLLOUT_unstructured==NULL){
      ROLLOUT_structured->open();
    }
  }
}

/* ------------------ ShowGluiTerrain ------------------------ */

extern "C" void ShowGluiTerrain(void){
  showterrain_dialog = 1;
  if(glui_geometry != NULL){
    glui_geometry->show();
    if(ROLLOUT_terrain != NULL){
      ROLLOUT_terrain->open();
    }
  }
}

/* ------------------ HideGluiTerrain ------------------------ */

extern "C" void HideGluiTerrain(void){
  showterrain_dialog = 0;
  CloseRollouts(glui_geometry);
  editwindow_status = CLOSE_WINDOW;
}

/* ------------------ UpdateBlockVals ------------------------ */

extern "C" void UpdateBlockVals(int flag){
  float xmin, xmax, ymin, ymax, zmin, zmax;
  int imin, jmin, kmin;
  int i;
  int temp;
  float *xplt_orig, *yplt_orig, *zplt_orig;
  int ibar, jbar, kbar;

  if(have_obsts==0)return;
  GetBlockVals(&xmin,&xmax,&ymin,&ymax,&zmin,&zmax,&imin,&jmin,&kmin);

  xplt_orig = current_mesh->xplt_orig;
  yplt_orig = current_mesh->yplt_orig;
  zplt_orig = current_mesh->zplt_orig;
  ibar = current_mesh->ibar;
  jbar = current_mesh->jbar;
  kbar = current_mesh->kbar;

  EDIT_xmin->set_float_limits(xplt_orig[0],xplt_orig[ibar],GLUI_LIMIT_CLAMP);
  EDIT_xmax->set_float_limits(xplt_orig[0],xplt_orig[ibar],GLUI_LIMIT_CLAMP);
  EDIT_ymin->set_float_limits(yplt_orig[0],yplt_orig[jbar],GLUI_LIMIT_CLAMP);
  EDIT_ymax->set_float_limits(yplt_orig[0],yplt_orig[jbar],GLUI_LIMIT_CLAMP);
  EDIT_zmin->set_float_limits(zplt_orig[0],zplt_orig[kbar],GLUI_LIMIT_CLAMP);
  EDIT_zmax->set_float_limits(zplt_orig[0],zplt_orig[kbar],GLUI_LIMIT_CLAMP);

  EDIT_xmin->set_float_val(xmin);
  EDIT_xmax->set_float_val(xmax);
  EDIT_ymin->set_float_val(ymin);
  EDIT_ymax->set_float_val(ymax);
  EDIT_zmin->set_float_val(zmin);
  EDIT_zmax->set_float_val(zmax);
  if(bchighlight!=NULL&&nsurfinfo>0){
    wall_case=bchighlight->walltype;
    ObjectCB(RADIO_WALL);
  }

  if(flag==SELECT_BLOCKS){
    if(bchighlight!=NULL){
      char dialog_label[255];
      char dialog_id[255];
      meshdata *blockmesh;

      if(nmeshes>1){
        blockmesh = meshinfo + bchighlight->meshindex;
//        sprintf(dialog_label,"Mesh label: %s",blockmesh->label);
        snprintf(dialog_label,sizeof(dialog_label),"Mesh label: %s",blockmesh->label);
        STATIC_mesh_index->set_text(dialog_label);
      }
//      sprintf(dialog_label,"&OBST index: %i",bchighlight->blockage_id);
      snprintf(dialog_label,sizeof(dialog_label),"&OBST index: %i",bchighlight->blockage_id);
      STATIC_blockage_index->set_text(dialog_label);
      strcpy(dialog_label,"&OBST label: ");
      strcat(dialog_label,bchighlight->label);
      STATIC_label->set_text(dialog_label);
      strcpy(dialog_id, "&OBST ID: ");
      strcat(dialog_id, bchighlight->id_label);
      STATIC_id_label->set_text(dialog_id);

      switch(wall_case){
      case WALL_1:
        temp=bchighlight->surf_index[UP_Z];
        for(i=0;i<6;i++){
          bchighlight->surf_index[i]=temp;
        }
        break;
      case WALL_3:
        temp=bchighlight->surf_index[UP_Y];
        bchighlight->surf_index[DOWN_X]=temp;
        bchighlight->surf_index[DOWN_Y]=temp;
        bchighlight->surf_index[UP_X]=temp;
        break;
      case WALL_6:
        break;
      default:
        assert(FFALSE);
        break;
      }

      if(nsurfinfo>0){
        for(i=0;i<6;i++){
          surface_indices[i] = inv_sorted_surfidlist[bchighlight->surf_index[i]];
          surface_indices_bak[i] = inv_sorted_surfidlist[bchighlight->surf_index[i]];
          LIST_obst_surface[i]->set_int_val(surface_indices[i]);
        }
      }
    }
    else{
      if(nsurfinfo>0){
        for(i=0;i<6;i++){
          surface_indices[i]=inv_sorted_surfidlist[0];
          surface_indices_bak[i]=inv_sorted_surfidlist[0];
          LIST_obst_surface[i]->set_int_val(surface_indices[i]);
        }
      }
    }
  }
}

/* ------------------ ObjectCB ------------------------ */

extern "C" void ObjectCB(int var){
  int i,temp;
  switch(var){
    case VISAXISLABELS:
      updatemenu=1;
      break;
    case UPDATE_LIST:
      switch(wall_case){
      case WALL_1:
        temp=surface_indices_bak[UP_Z];
        if(nsurfinfo>0){
          for(i=0;i<6;i++){
            surface_indices[i]=temp;
            LIST_obst_surface[i]->set_int_val(temp);
          }
        }
        break;
      case WALL_3:
        if(nsurfinfo>0){
          for(i=0;i<6;i++){
            temp=surface_indices_bak[i];
            surface_indices[i]=temp;
            LIST_obst_surface[i]->set_int_val(temp);
          }
        }
        break;
      case WALL_6:
        if(nsurfinfo>0){
          for(i=0;i<6;i++){
            temp=surface_indices_bak[i];
            surface_indices[i]=temp;
            LIST_obst_surface[i]->set_int_val(temp);
          }
        }
        break;
      default:
        assert(FFALSE);
        break;
      }

      if(bchighlight!=NULL){
        for(i=0;i<6;i++){
          bchighlight->surf[i]=surfinfo+sorted_surfidlist[surface_indices_bak[i]];
          bchighlight->surf_index[i]=sorted_surfidlist[surface_indices_bak[i]];
        }
        bchighlight->changed_surface=1;
        if(bchighlight->blockage_id>0&&bchighlight->blockage_id<=nchanged_idlist){
          changed_idlist[bchighlight->blockage_id]=1;
        }
        blockages_dirty=1;
        UpdateUseTextures();
        UpdateFaces();
      }
      break;
    case RADIO_WALL:
      if(nsurfinfo==0)break;
      if(bchighlight!=NULL){
        bchighlight->walltype=wall_case;
      }
      switch(wall_case){
      case WALL_6:
        for(i=0;i<6;i++){
          LIST_obst_surface[i]->enable();
        }
        LIST_obst_surface[DOWN_Z]->set_name("z lower face");
        LIST_obst_surface[UP_Z]->set_name("z upper face");
        LIST_obst_surface[DOWN_Y]->set_name("y lower face");
        LIST_obst_surface[UP_Y]->set_name("y upper face");
        LIST_obst_surface[DOWN_X]->set_name("x lower face");
        LIST_obst_surface[UP_X]->set_name("x upper face");
        break;
      case WALL_3:
        for(i=0;i<6;i++){
          LIST_obst_surface[i]->disable();
        }
        LIST_obst_surface[DOWN_Z]->enable();
        LIST_obst_surface[UP_Z]->enable();
        LIST_obst_surface[UP_Y]->enable();

        LIST_obst_surface[DOWN_Z]->set_name("z lower face");
        LIST_obst_surface[UP_Z]->set_name("z upper face");
        LIST_obst_surface[UP_Y]->set_name("side faces");
        LIST_obst_surface[DOWN_Y]->set_name("");
        LIST_obst_surface[DOWN_X]->set_name("");
        LIST_obst_surface[UP_X]->set_name("");

        break;
      case WALL_1:
        for(i=0;i<6;i++){
          LIST_obst_surface[i]->disable();
        }
        LIST_obst_surface[UP_Z]->enable();
        LIST_obst_surface[UP_Z]->set_name("All faces");

        LIST_obst_surface[DOWN_Z]->set_name("");
        LIST_obst_surface[DOWN_Y]->set_name("");
        LIST_obst_surface[UP_Y]->set_name("");
        LIST_obst_surface[DOWN_X]->set_name("");
        LIST_obst_surface[UP_X]->set_name("");
        break;
      default:
        assert(FFALSE);
        break;
      }
      ObjectCB(UPDATE_LIST);
      break;
      case BLOCKAGE_AS_INPUT2:
      case BLOCKAGE_AS_INPUT:
        if(var==BLOCKAGE_AS_INPUT2){
          blockage_snapped=1-blockage_as_input;
          if(CHECKBOX_blockage!=NULL)CHECKBOX_blockage->set_int_val(blockage_snapped);
        }
        blockage_as_input=1-blockage_snapped;
        if(blocklocation!=BLOCKlocation_cad){
          if(blockage_as_input==1){
            blocklocation=BLOCKlocation_exact;
          }
          else{
            blocklocation=BLOCKlocation_grid;
          }
        }
        UpdateBlockVals(NOT_SELECT_BLOCKS);
        break;
    default:
      assert(FFALSE);
      break;
  }
}
