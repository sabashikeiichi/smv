#include "options.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "histogram.h"
#include "compress.h"
#include "IOobjects.h"
#include "stdio_m.h"

#define NXYZ_COMP_PART 3

#define LOADING 0
#define DRAWING 1

#define FORTPART5READ_mv(var,size) \
fseek_m(PART5FILE,4,SEEK_CUR);\
fread_mv(var,4,size,PART5FILE);\
fseek_m(PART5FILE,4,SEEK_CUR);\
returncode=feof_m(PART5FILE)

#define FORTPART5READ_m(var,size) \
fseek_m(PART5FILE,4,SEEK_CUR);\
fread_m(var,4,size,PART5FILE);\
fseek_m(PART5FILE,4,SEEK_CUR);\
returncode = feof_m(PART5FILE)

#define FCLOSE_m(a) fclose_m((a))

#define FSEEK_m(a,b,c) \
  fseek_m((a),(b),(c));\
  returncode = feof_m(PART5FILE)

#define FREAD_m(a,b,c,d) fread_m((a),(b),(c),(d))

#define FORTPART5READ(var,size) \
returncode=PASS_m;\
FSEEK(PART5FILE,4,SEEK_CUR);if(ferror(PART5FILE)==1||feof(PART5FILE)==1)returncode=FAIL_m;\
if(returncode==PASS_m){\
  fread(var,4,size,PART5FILE);\
  if(ferror(PART5FILE)==1||feof(PART5FILE)==1)returncode=FAIL_m;\
}\
if(returncode==PASS_m){\
  FSEEK(PART5FILE,4,SEEK_CUR);\
  if(ferror(PART5FILE)==1||feof(PART5FILE)==1)returncode=FAIL_m;\
}

/* ------------------ ClosePartFiles ------------------------ */

void ClosePartFiles(void){
  int i;

  for(i = 0; i<npartinfo; i++){
    partdata *parti;

    parti = partinfo+i;
    if(parti->loaded==1&&parti->stream!=NULL){
      FCLOSE_m(parti->stream);
      parti->stream = NULL;
    }
  }
}

/* ------------------ GetPartColor ------------------------ */

int GetPartColor(float **color_handle, part5data*datacopy, int show_default, int j, int itype,
                     float valmin, float valmax){
  float *colorptr;
  unsigned char *color;
  int showcolor;

  showcolor = 1;
  if(show_default == 1){
    colorptr = datacopy->partclassbase->rgb;
  }
  else{
    float *rvals;

    color = datacopy->irvals + itype*datacopy->npoints;
    rvals = datacopy->rvals + itype*datacopy->npoints;
    {
     int colorj;
      float rval;

      rval = CLAMP(255.0*(rvals[j]-valmin)/(valmax-valmin), 0.0, 255.0);
      colorj = rval;
      colorptr = rgb_part + 4*colorj;
    }
    if(current_property != NULL && (color[j] > current_property->imax || color[j] < current_property->imin))showcolor = 0;
  }
  *color_handle = colorptr;
  return showcolor;
}

/* ------------------ GetPartPropS ------------------------ */

partpropdata *GetPartPropS(char *label){
  int i;

  for(i = 0;i < npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->shortlabel, label) == 0)return propi;
  }
  return NULL;
}

/* ------------------ CopyDepVals ------------------------ */

void CopyDepVals(partclassdata *partclassi, part5data *datacopy, float *colorptr, propdata *prop, int j){
  int ii;
  int ndep_vals;
  float *dep_vals;

  if(prop == NULL)return;
  dep_vals = partclassi->fvars_dep;
  ndep_vals = partclassi->nvars_dep;
  for(ii = 0; ii < partclassi->nvars_dep - 3; ii++){

    unsigned char *var_type;
    unsigned char color_index;
    partpropdata *varprop;
    float valmin, valmax;
    char *shortlabel;
    flowlabels *label;

    shortlabel = NULL;
    varprop = NULL;
    label = datacopy->partclassbase->labels + ii + 2;
    if(label != NULL)shortlabel = label->shortlabel;
    if(shortlabel != NULL)varprop = GetPartPropS(shortlabel);
    if(varprop != NULL){
      var_type = datacopy->irvals + ii*datacopy->npoints;
      color_index = var_type[j];
      valmin = varprop->valmin;
      valmax = varprop->valmax;
      dep_vals[ii] = valmin + color_index*(valmax - valmin) / 255.0;
    }
    else{
      dep_vals[ii] = 1.0;
    }
  }

  dep_vals[ndep_vals - 3] = colorptr[0] * 255;
  dep_vals[ndep_vals - 2] = colorptr[1] * 255;
  dep_vals[ndep_vals - 1] = colorptr[2] * 255;
  prop->nvars_dep = partclassi->nvars_dep;
  prop->smv_object->visible = 1;
  for(ii = 0; ii < prop->nvars_dep; ii++){
    prop->fvars_dep[ii] = partclassi->fvars_dep[ii];
  }
  prop->nvars_dep = partclassi->nvars_dep;
  for(ii = 0; ii < partclassi->nvars_dep; ii++){
    prop->vars_dep_index[ii] = partclassi->vars_dep_index[ii];
  }
  prop->tag_number = datacopy->tags[j];
}

/* ------------------ CompareTags ------------------------ */

int CompareTags(const void *arg1, const void *arg2){
  int i, j;

  i = *(int *)arg1;
  j = *(int *)arg2;
  if(i < j)return -1;
  if(i > j)return 1;
  return 0;

}

/* ------------------ GetTagIndex ------------------------ */

int GetTagIndex(const partdata *partin_arg, part5data **datain_arg, int tagval_arg, int flag){
  int *returnval_local;
  part5data *data_local;
  int i;

  if(flag==LOADING&&partfast==YES)return -1;

  for(i = -1; i < npartinfo; i++){
    const partdata *parti_local;

    if(i == -1){
      parti_local = partin_arg;
      data_local = *datain_arg;
    }
    else{
      parti_local = partinfo + i;
      if(parti_local== partin_arg)continue;
      if(parti_local->loaded == 0 || parti_local->display == 0)continue;
      data_local = parti_local->data5 + (*datain_arg- partin_arg->data5);
    }
    if(parti_local->loaded == 0 || parti_local->display == 0)continue;

    if(data_local->npoints == 0)continue;
    assert(data_local->npoints>0);
    assert(data_local->sort_tags != NULL);
    returnval_local = bsearch(&tagval_arg, data_local->sort_tags, data_local->npoints, 2 * sizeof(int), CompareTags);
    if(returnval_local== NULL)continue;
    *datain_arg = data_local;
    return *(returnval_local+ 1);
  }
  return -1;
}

/* ------------------ GetPartBounds ------------------------ */

void GetPartBounds(float *valmin, float *valmax){
  *valmin = part5propinfo[global_prop_index].ppartlevels256[0];
  *valmax = part5propinfo[global_prop_index].ppartlevels256[255];
}

/* ------------------ DrawPart ------------------------ */

void DrawPart(const partdata *parti){
  int ipframe;
  part5data *datacopy, *datapast;
  int nclasses;
  int i, j;
  int offset_terrain;
  propdata *prop;
  float valmin, valmax;

  if(nglobal_times<1||parti->times[0] > global_times[itimes])return;
  if(nterraininfo > 0 && ABS(vertical_factor - 1.0) > 0.01){
    offset_terrain = 1;
  }
  else{
    offset_terrain = 0;
  }

  if(current_property == NULL)return;
  GetPartBounds(&valmin, &valmax);
  if(valmin>=valmax){
    valmin = 0.0;
    valmax = 1.0;
  }
  ipframe = parti->itime;
  assert(ipframe>=0);
  if(ipframe < 0){
    ipframe = 0;
  } //xxx need to check this - why is ipframe < 0 ???
  nclasses = parti->nclasses;
  datacopy = parti->data5 + nclasses*ipframe;
  CheckMemory;
  if(part5show == 1){
    if(streak5show == 0 || (streak5show == 1 && showstreakhead == 1)){
      for(i = 0;i < parti->nclasses;i++){
        short *sx, *sy, *sz;
        unsigned char *vis, *color;
        partclassdata *partclassi;
        int partclass_index, itype, vistype, class_vis;
        int show_default;

        partclassi = parti->partclassptr[i];
        partclass_index = partclassi - partclassinfo;

        vistype = current_property->class_present[partclass_index];
        class_vis = current_property->class_vis[partclass_index];

        if(vistype == 0 || datacopy->npoints <= 0 || (vistype == 1 && class_vis == 0)){
          if(show_tracers_always == 0 || partclassi->ntypes > 2){
            datacopy++;
            continue;
          }
        }
        itype = current_property->class_types[partclass_index];

        show_default = 0;
        if(itype == -1 || (show_tracers_always == 1 && partclassi->ntypes <= 2)){
          show_default = 1;
        }

        sx = datacopy->sx;
        sy = datacopy->sy;
        sz = datacopy->sz;
        vis = datacopy->vis_part;
        {
          glPointSize(partpointsize);
          if(offset_terrain == 0){

            // *** draw particles as points

            if(datacopy->partclassbase->vis_type == PART_POINTS){
              glBegin(GL_POINTS);
              if(show_default == 1){
                glColor4fv(datacopy->partclassbase->rgb);
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
                  }
                }
              }
              else{
                float *rvals;

                rvals = datacopy->rvals+itype*datacopy->npoints;
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    int colorj;
                    float rval;
                    rval = CLAMP(255.0*(rvals[j]-valmin)/(valmax-valmin), 0.0, 255.0);
                    colorj = rval;
                    if(current_property != NULL && (colorj > current_property->imax || colorj < current_property->imin))continue;
                    if(rgb_part[4*colorj+3]==0.0)continue;
                    glColor4fv(rgb_part+4*colorj);
                    glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
                  }
                }
              }
              glEnd();
            }

            // *** draw particles using smokeview object

            if(datacopy->partclassbase->vis_type == PART_SMV_DEVICE){
              for(j = 0;j < datacopy->npoints;j++){
                float *colorptr;

                if(vis[j] != 1)continue;

                glPushMatrix();
                glTranslatef(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);

                glRotatef(-datacopy->partclassbase->elevation, 0.0, 1.0, 0.0);
                glRotatef(datacopy->partclassbase->azimuth, 0.0, 0.0, 1.0);

                //  0->2   color
                //  3      diameter
                //  4      length

                if(show_default == 1){
                  colorptr = datacopy->partclassbase->rgb;
                }
                else{
                  float *rvals, rval;

                  rvals = datacopy->rvals+itype*datacopy->npoints;
                  rval = CLAMP(255.0*(rvals[j]-valmin)/(valmax-valmin), 0.0, 255.0);
                  colorptr = rgb_full[(int)rval];
                }
                prop = datacopy->partclassbase->prop;
                CopyDepVals(partclassi, datacopy, colorptr, prop, j);
                glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));

                partfacedir[0] = xbar0 + SCALE2SMV(fds_eyepos[0]) - xplts[sx[j]];
                partfacedir[1] = ybar0 + SCALE2SMV(fds_eyepos[1]) - yplts[sy[j]];
                partfacedir[2] = zbar0 + SCALE2SMV(fds_eyepos[2]) - zplts[sz[j]];

                DrawSmvObject(prop->smv_object, 0, prop, 0, NULL, 0);
                glPopMatrix();
              }
            }

            // *** draw particle as lines

            if(datacopy->partclassbase->vis_type == PART_LINES
              && ((datacopy->dsx != NULL&&datacopy->dsy != NULL&&datacopy->dsz != NULL) || datacopy->partclassbase->device_name != NULL)
              ){
              float *dxv, *dyv, *dzv;
              float dx, dy, dz;
              int flag = 0;

              if(datacopy->dsx != NULL&&datacopy->dsy != NULL&&datacopy->dsz != NULL){
                flag = 1;
                dxv = datacopy->dsx;
                dyv = datacopy->dsy;
                dzv = datacopy->dsz;
              }
              else{
                dx = datacopy->partclassbase->dx;
                dy = datacopy->partclassbase->dy;
                dz = datacopy->partclassbase->dz;
              }
              glBegin(GL_LINES);
              if(show_default == 1){
                glColor4fv(datacopy->partclassbase->rgb);
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    if(flag == 1){
                      dx = dxv[j];
                      dy = dyv[j];
                      dz = dzv[j];
                    }
                    glVertex3f(xplts[sx[j]] - dx, yplts[sy[j]] - dy, zplts[sz[j]] - dz);
                    glVertex3f(xplts[sx[j]] + dx, yplts[sy[j]] + dy, zplts[sz[j]] + dz);
                  }
                }
              }
              else{
                color = datacopy->irvals + itype*datacopy->npoints;
                for(j = 0;j < datacopy->npoints;j++){
                  if(vis[j] == 1){
                    glColor4fv(rgb_full[color[j]]);
                    if(flag == 1){
                      dx = dxv[j];
                      dy = dyv[j];
                      dz = dzv[j];
                    }
                    glVertex3f(xplts[sx[j]] - dx, yplts[sy[j]] - dy, zplts[sz[j]] - dz);
                    glVertex3f(xplts[sx[j]] + dx, yplts[sy[j]] + dy, zplts[sz[j]] + dz);
                  }
                }
              }
              glEnd();
            }
          }
          else{
            glBegin(GL_POINTS);
            if(show_default == 1){
              glColor4fv(datacopy->partclassbase->rgb);
              for(j = 0;j < datacopy->npoints;j++){
                float zoffset;
                float xx, yy, zz;
                int loc;

                xx = xplts[sx[j]];
                yy = yplts[sy[j]];
                zz = zplts[sz[j]];

                zoffset = GetZCellValOffset(meshinfo, xx, yy, &loc);
                if(vis[j] == 1)glVertex3f(xx, yy, zz + zoffset);
              }
            }
            else{
              color = datacopy->irvals + itype*datacopy->npoints;
              for(j = 0;j < datacopy->npoints;j++){
                if(vis[j] == 1){
                  glColor4fv(rgb_full[color[j]]);
                  glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
                }
              }
            }
            glEnd();
          }
        }

        datacopy++;
      }
    }
  }

  // draw streak lines

  datacopy = parti->data5 + nclasses*ipframe;

  if(streak5show == 1){
    for(i = 0;i < parti->nclasses;i++){
      short *sx, *sy, *sz;
      short *sxx, *syy, *szz;
      unsigned char *vis;
      int k;
      int show_default;
      float *colorptr;

      partclassdata *partclassi;
      int partclass_index, itype, vistype, class_vis;

      partclassi = parti->partclassptr[i];
      partclass_index = partclassi - partclassinfo;

      vistype = current_property->class_present[partclass_index];
      class_vis = current_property->class_vis[partclass_index];

      if(vistype == 0 || datacopy->npoints <= 0 || (vistype == 1 && class_vis == 0)){
        if(show_tracers_always == 0 || partclassi->ntypes > 2){
          datacopy++;
          continue;
        }
      }
      itype = current_property->class_types[partclass_index];

      show_default = 0;
      if(itype == -1 || (show_tracers_always == 1 && partclassi->ntypes <= 2)){
        show_default = 1;
      }

      sx = datacopy->sx;
      sy = datacopy->sy;
      sz = datacopy->sz;
      vis = datacopy->vis_part;

      if(show_default == 1){

        // draw the streak line

        GetPartColor(&colorptr, datacopy, show_default, 0, itype, valmin, valmax);
        glColor4fv(colorptr);

        glLineWidth(streaklinewidth);
        for(j = 0;j < datacopy->npoints;j++){
          int tagval;

          tagval = datacopy->tags[j];
          if(vis[j] == 0)continue;
          glBegin(GL_LINE_STRIP);
          glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
          for(k = 1;k < streak5step;k++){
            int jj;

            if(ipframe - k < 0)break;
            datapast = parti->data5 + nclasses*(ipframe - k) + i;
            jj = GetTagIndex(parti, &datapast, tagval, DRAWING);
            if(jj < 0)break;
            sxx = datapast->sx;
            syy = datapast->sy;
            szz = datapast->sz;
            glVertex3f(xplts[sxx[jj]], yplts[syy[jj]], zplts[szz[jj]]);
          }
          glEnd();
        }

        // draw the dot at the end of the streak line
      }
      else{

        // draw the streak line

        for(j = 0;j < datacopy->npoints;j++){
          int tagval;

          tagval = datacopy->tags[j];
          if(vis[j] == 0)continue;
          if(GetPartColor(&colorptr, datacopy, show_default, j, itype, valmin, valmax)==0)continue;
          if(colorptr[3]==0.0)continue;

          glBegin(GL_LINE_STRIP);
          glColor4fv(colorptr);
          glVertex3f(xplts[sx[j]], yplts[sy[j]], zplts[sz[j]]);
          for(k = 1;k < streak5step;k++){
            int jj;

            if(ipframe - k < 0)break;
            datapast = parti->data5 + nclasses*(ipframe - k) + i;
            jj = GetTagIndex(parti, &datapast, tagval, DRAWING);
            if(jj < 0 || datapast->irvals == NULL)break;
            sxx = datapast->sx;
            syy = datapast->sy;
            szz = datapast->sz;

            GetPartColor(&colorptr, datacopy, show_default, jj, itype, valmin, valmax);
            glColor4fv(colorptr);
            glVertex3f(xplts[sxx[jj]], yplts[syy[jj]], zplts[szz[jj]]);
          }
          glEnd();
        }
      }

      datacopy++;
    }
  }

}

/* ------------------ DrawPartFrame ------------------------ */

void DrawPartFrame(void){
  partdata *parti;
  int i;

  if(use_tload_begin==1&&global_times[itimes]<tload_begin)return;
  if(use_tload_end==1&&global_times[itimes]>tload_end)return;
  for(i=0;i<npartinfo;i++){
    parti = partinfo + i;
    if(parti->loaded==0||parti->display==0)continue;
    DrawPart(parti);
    SNIFF_ERRORS("after DrawPart");
  }
}

/* ------------------ FreePart5Data ------------------------ */

void FreePart5Data(part5data *datacopy_arg){
  FREEMEMORY(datacopy_arg->cvals);
  FREEMEMORY(datacopy_arg->dsx);
  FREEMEMORY(datacopy_arg->dsy);
  FREEMEMORY(datacopy_arg->dsz);
  FREEMEMORY(datacopy_arg->avatar_angle);
  FREEMEMORY(datacopy_arg->avatar_width);
  FREEMEMORY(datacopy_arg->avatar_height);
  FREEMEMORY(datacopy_arg->avatar_depth);
}

/* ------------------ FreeAllPart5Data ------------------------ */

void FreeAllPart5Data(partdata *parti){
  int i;
  part5data *datacopy_local;

  if(parti->data5==NULL)return;
  datacopy_local = parti->data5;
  for(i=0;i<parti->ntimes*parti->nclasses;i++){
    FreePart5Data(datacopy_local);
    datacopy_local++;
  }
  FREEMEMORY(parti->data5);
  FREEMEMORY(parti->vis_part);
  FREEMEMORY(parti->tags);
  FREEMEMORY(parti->sort_tags);
  FREEMEMORY(parti->sx);
  FREEMEMORY(parti->sy);
  FREEMEMORY(parti->sz);
  FREEMEMORY(parti->irvals);
}

/* ------------------ InitPart5Data ------------------------ */

void InitPart5Data(part5data *datacopy, partclassdata *partclassi){
  datacopy->cvals=NULL;
  datacopy->partclassbase=partclassi;
  datacopy->sx=NULL;
  datacopy->sy=NULL;
  datacopy->sz=NULL;
  datacopy->dsx=NULL;
  datacopy->dsy=NULL;
  datacopy->dsz=NULL;
  datacopy->avatar_angle=NULL;
  datacopy->avatar_width=NULL;
  datacopy->avatar_height=NULL;
  datacopy->avatar_depth=NULL;
  datacopy->tags=NULL;
  datacopy->vis_part=NULL;
  datacopy->sort_tags=NULL;
  datacopy->rvals=NULL;
  datacopy->irvals=NULL;
}

/* ------------------ ComparePart ------------------------ */

int ComparePart(const void *arg1, const void *arg2){
  partdata *parti, *partj;
  int i, j;

  i = *(int *)arg1;
  j = *(int *)arg2;

  parti = partinfo + i;
  partj = partinfo + j;

  if(parti->blocknumber<partj->blocknumber)return -1;
  if(parti->blocknumber>partj->blocknumber)return 1;
  return 0;
}

/* ------------------ UpdatePartVis ------------------------ */

void UpdatePartVis(int first_frame_arg, partdata *parti_arg, part5data *datacopy_arg, int nclasses_arg){
  int nparts_local;
  unsigned char *vis_part_local;

  nparts_local = datacopy_arg->npoints;
  vis_part_local = datacopy_arg->vis_part;

  if(first_frame_arg== 1){
    int ii;

    for(ii = 0; ii < nparts_local; ii++){
      vis_part_local[ii] = 1;
    }
  }
  else{
    int ii;

    part5data *datalast_local;
    int nvis_local = 0, nleft_local;

    for(ii = 0; ii < nparts_local; ii++){
      int tag_index_local;

      datalast_local = datacopy_arg- nclasses_arg;
      tag_index_local = GetTagIndex(parti_arg, &datalast_local, datacopy_arg->tags[ii], LOADING);
      if(tag_index_local!= -1 && datalast_local->vis_part[tag_index_local] == 1){
        datacopy_arg->vis_part[ii] = 1;
        nvis_local++;
      }
      else{
        datacopy_arg->vis_part[ii] = 0;
      }
    }

    nleft_local = nparts_local- nvis_local;
    if(nleft_local > 0){
      for(ii = 0; ii<nparts_local; ii++){
        if(datacopy_arg->vis_part[ii] == 1)continue;
        if(nleft_local>0){
          datacopy_arg->vis_part[ii] = 1;
          nleft_local--;
        }
      }
    }
  }
}

/* ------------------ UpdateAllPartVis ------------------------ */

void UpdateAllPartVis(partdata *parti){
  part5data *datacopy_local;
  int i, j;
  int firstframe_local = 1;

  datacopy_local = parti->data5;
  for(i = 0; i < parti->ntimes; i++){
    for(j = 0; j < parti->nclasses; j++){
      UpdatePartVis(firstframe_local, parti, datacopy_local, parti->nclasses);
      datacopy_local++;
    }
    if(firstframe_local== 1)firstframe_local = 0;
  }
}

/* ------------------ GetHistFileStatus ------------------------ */

#ifdef pp_HIST
int GetHistFileStatus(partdata *parti){

  // return -1 if history file cannot be created (corresponding particle file does not exist)
  // return  0 if history file does not need to be created
  // return  1 if history file needs to be created (doesn't exist or is older than corresponding particle file)

  STRUCTSTAT stat_histfile_buffer, stat_regfile_buffer;
  int stat_histfile, stat_regfile;

  stat_histfile = STAT(parti->hist_file, &stat_histfile_buffer);
  stat_regfile = STAT(parti->reg_file, &stat_regfile_buffer);

  if(stat_regfile != 0)return HIST_ERR;                    // particle filei does not exist

  if(stat_regfile_buffer.st_size > parti->reg_file_size){  // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    return HIST_OLD;
  }

  if(stat_histfile != 0) return HIST_OLD;                  // history file does not exist
  if(stat_regfile_buffer.st_mtime > stat_histfile_buffer.st_mtime)return HIST_OLD; // size file is older than particle file
  return HIST_OK;
}
#endif

/* ------------------ GetSizeFileStatus ------------------------ */

int GetSizeFileStatus(partdata *parti){

  // return -1 if size file cannot be created (corresponding particle file does not exist)
  // return  0 if size file does not need to be created
  // return  1 if size file needs to be created (doesn't exist or is older than corresponding particle file)

  STRUCTSTAT stat_sizefile_buffer, stat_regfile_buffer;
  int stat_sizefile, stat_regfile;

  stat_sizefile = STAT(parti->size_file, &stat_sizefile_buffer);
  stat_regfile = STAT(parti->reg_file, &stat_regfile_buffer);

  if(stat_regfile != 0)return -1;                         // particle file does not exist

  if(stat_sizefile != 0) return 1;                        // size file does not exist
  if(stat_regfile_buffer.st_size > parti->reg_file_size){ // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    return 1;
  }
  if(stat_regfile_buffer.st_mtime > stat_sizefile_buffer.st_mtime)return 1; // size file is older than particle file
  return 0;
}

/* ------------------ CreatePartSizeFileFromBound ------------------------ */

void CreatePartSizeFileFromBound(char *part5boundfile_arg, char *part5sizefile_arg, LINT filepos_arg){
  FILE *streamin_local, *streamout_local;
  float time_local;
  char buffer_local[255];

  streamin_local = fopen(part5boundfile_arg, "r");
  streamout_local = fopen(part5sizefile_arg, "w");

  for(;;){
    int nclasses_local,j,eof_local;
    LINT frame_size_local;
    char format[128];

    eof_local = 0;
    frame_size_local =0;
    if(fgets(buffer_local,255,streamin_local)==NULL)break;
    sscanf(buffer_local, "%f %i", &time_local, &nclasses_local);
#ifdef INTEL_WIN_COMPILER
      strcpy(format, " %f %lli");
#else
      strcpy(format, " %f %li");
#endif
    fprintf(streamout_local, format, time_local, filepos_arg);
    fprintf(streamout_local, "\n");
    frame_size_local += 12;
    for(j = 0; j<nclasses_local; j++){
      int k, npoints_local, ntypes_local;

      frame_size_local += 12;
      if(fgets(buffer_local, 255, streamin_local)==NULL){
        eof_local = 1;
        break;
      }
      sscanf(buffer_local, "%i %i", &ntypes_local, &npoints_local);
      frame_size_local += 4+4*NXYZ_COMP_PART*npoints_local+4;
      frame_size_local += 4 + 4*npoints_local+ 4;
      if(ntypes_local>0){
        frame_size_local += 4 + 4*npoints_local*ntypes_local+ 4;
      }
      fprintf(streamout_local, " %i\n", npoints_local);
      for(k = 0; k<ntypes_local; k++){
        if(fgets(buffer_local, 255, streamin_local)==NULL){
          eof_local = 1;
          break;
        }
      }
      if(eof_local==1)break;
    }
    if(eof_local==1)break;
    filepos_arg += frame_size_local;
  }
  fclose(streamin_local);
  fclose(streamout_local);
}

/* ------------------ CreatePartSizeFileFromPart ------------------------ */

void CreatePartSizeFileFromPart(char *part5file_arg, char *part5sizefile_arg, LINT file_offset_arg){
  FILE *PART5FILE, *streamout_local;
  int returncode=0;
  int one_local, version_local, nclasses_local=0;
  int i;
  int *numtypes_local, *numpoints_local;
  int skip_local, numvals_local;

  PART5FILE = fopen(part5file_arg, "rb");
  streamout_local = fopen(part5sizefile_arg, "w");

  FSEEK(PART5FILE, 4, SEEK_CUR); fread(&one_local, 4, 1, PART5FILE); FSEEK(PART5FILE, 4, SEEK_CUR);
  FORTPART5READ(&version_local, 1);

  FORTPART5READ(&nclasses_local, 1);
  NewMemory((void **)&numtypes_local, 2*nclasses_local* sizeof(int));
  NewMemory((void **)&numpoints_local, nclasses_local* sizeof(int));
  for (i = 0; i < nclasses_local; i++){
    FORTPART5READ(numtypes_local+2*i, 2);
    numvals_local = numtypes_local[2 * i] + numtypes_local[2 * i + 1];
    skip_local = 2*numvals_local*(4 + 30 + 4);
    FSEEK(PART5FILE, skip_local, SEEK_CUR);
  }
  while (!feof(PART5FILE)){
    float time_local;
    LINT frame_size_local;
    char format[128];

    frame_size_local =0;

    FORTPART5READ(&time_local, 1);
    frame_size_local += 12;
    if(returncode == FAIL_m)break;
    for (i = 0; i < nclasses_local; i++){
      FORTPART5READ(numpoints_local+ i, 1);
      frame_size_local += 12;
      skip_local = 4+4*NXYZ_COMP_PART*numpoints_local[i]+4;
      skip_local += 4 + 4 * numpoints_local[i] + 4;
      if(numtypes_local[2 * i] > 0)    skip_local += 4 + 4 * numpoints_local[i] * numtypes_local[2 * i] + 4;
      if(numtypes_local[2 * i + 1] > 0)skip_local += 4 + 4 * numpoints_local[i] * numtypes_local[2 * i + 1] + 4;
      FSEEK(PART5FILE, skip_local, SEEK_CUR);
      frame_size_local +=skip_local;
    }
#ifdef INTEL_WIN_COMPILER
    strcpy(format, "%f %lli");
#else
    strcpy(format, "%f %li");
#endif
    fprintf(streamout_local, format, time_local, file_offset_arg);
    fprintf(streamout_local, "\n");
    file_offset_arg += frame_size_local;
    for (i = 0; i < nclasses_local; i++){
      fprintf(streamout_local, " %i\n", numpoints_local[i]);
    }
  }
  fclose(PART5FILE);
  fclose(streamout_local);
  FREEMEMORY(numtypes_local);
  FREEMEMORY(numpoints_local);
}

/* ------------------ GetPartHeaderOffset ------------------------ */

LINT GetPartHeaderOffset(partdata *parti_arg){
  LINT file_offset_local=0;
  int nclasses_local, one_local;
  FILE *PART5FILE=NULL;
  int version_local;
  int returncode=0;
  int skip_local;
  int i;
  int *numtypes_local = NULL, *numtypescopy_local, *numpoints_local = NULL;
  int numtypes_temp_local[2];

  PART5FILE = fopen(parti_arg->reg_file,"rb");
  if(PART5FILE==NULL)return 0;

  FSEEK(PART5FILE,4,SEEK_CUR);fread(&one_local,4,1,PART5FILE);FSEEK(PART5FILE,4,SEEK_CUR);
  file_offset_local += 12;

  FORTPART5READ(&version_local, 1);
  if(returncode == FAIL_m)goto wrapup;
  file_offset_local += 12;

  FORTPART5READ(&nclasses_local,1);
  if(returncode == FAIL_m)goto wrapup;
  file_offset_local += 12;

  NewMemory((void **)&numtypes_local,2*nclasses_local*sizeof(int));
  NewMemory((void **)&numpoints_local,nclasses_local*sizeof(int));
  numtypescopy_local =numtypes_local;
  numtypes_temp_local[0]=0;
  numtypes_temp_local[1]=0;
  CheckMemory;
  for(i=0;i<nclasses_local;i++){
    FORTPART5READ(numtypes_temp_local,2);
    file_offset_local += 16;
    if(returncode == FAIL_m)goto wrapup;
    *numtypescopy_local++=numtypes_temp_local[0];
    *numtypescopy_local++=numtypes_temp_local[1];
    skip_local = 2*(numtypes_temp_local[0]+numtypes_temp_local[1])*(8 + 30);
    file_offset_local +=skip_local;
    returncode=FSEEK(PART5FILE,skip_local,SEEK_CUR);
    if(returncode == FAIL_m)goto wrapup;
  }
  CheckMemory;

  wrapup:
  FREEMEMORY(numtypes_local);
  FREEMEMORY(numpoints_local);
  fclose(PART5FILE);
  return file_offset_local;
}

/* ------------------ CreatePartBoundFile ------------------------ */

void CreatePartBoundFile(partdata *parti){
  FILE_m *PART5FILE=NULL;
  int one_local, version_local, nclasses_local;
  int i;
  size_t returncode;
  float time_local;
  int nparts_local, *numtypes_local = NULL, numtypes_temp_local[2];
  FILE *stream_out_local=NULL;

  
  if(parti->reg_file!=NULL)PART5FILE = fopen_m(parti->reg_file, "rbm");
  if(parti->reg_file==NULL||PART5FILE==NULL)return;
  if(parti->bound_file!=NULL)stream_out_local = fopen(parti->bound_file, "w");
  if(stream_out_local==NULL){
    FCLOSE_m(PART5FILE);
    return;
  }

  FSEEK_m(PART5FILE, 4, SEEK_CUR);
  FREAD_m(&one_local, 4, 1, PART5FILE);
  FSEEK_m(PART5FILE, 4, SEEK_CUR);

  FORTPART5READ_m(&version_local, 1);
  if(returncode == FAIL_m)goto wrapup;

  FORTPART5READ_m(&nclasses_local, 1);
  if(returncode == FAIL_m)goto wrapup;

  NewMemory((void **)&numtypes_local, 2*nclasses_local*sizeof(int));
  numtypes_temp_local[0] = 0;
  numtypes_temp_local[1] = 0;
  CheckMemory;
  for(i = 0; i<nclasses_local; i++){
    int skip_local;

    FORTPART5READ_m(numtypes_temp_local, 2);
    if(returncode == FAIL_m)goto wrapup;
    numtypes_local[2*i+0] = numtypes_temp_local[0];
    numtypes_local[2*i+1] = numtypes_temp_local[1];
    skip_local = 2*(numtypes_temp_local[0]+numtypes_temp_local[1])*(8+30);
    FSEEK_m(PART5FILE, skip_local, SEEK_CUR);
    if(returncode == FAIL_m){
      goto wrapup;
    }
  }
  CheckMemory;

  for(;;){
    int jj;

    CheckMemory;
    FORTPART5READ_m(&time_local, 1);
    if(returncode == FAIL_m)goto wrapup;
    fprintf(stream_out_local, "%f %i 1\n", time_local, nclasses_local);

    for(jj = 0; jj<nclasses_local; jj++){
      int skip_local, kk;
      float *rvals_local;

      FORTPART5READ_m(&nparts_local, 1);
      if(returncode == FAIL_m)goto wrapup;

      fprintf(stream_out_local, "%i %i\n", numtypes_local[2*jj], nparts_local);
      CheckMemory;

      skip_local = 4+NXYZ_COMP_PART*nparts_local*sizeof(float)+4; // skip over particle xyz coords
      skip_local += 4+nparts_local*sizeof(int)+4;                   // skip over tags
      FSEEK_m(PART5FILE, skip_local, SEEK_CUR);
      if(returncode == FAIL_m)goto wrapup;
      CheckMemory;
      if(numtypes_local[2*jj]>0){
        FORTPART5READ_mv((void **)&rvals_local, nparts_local*numtypes_local[2*jj]);
        if(returncode == FAIL_m)goto wrapup;
      }
      CheckMemory;
      for(kk = 0; kk<numtypes_local[2*jj]; kk++){
        if(nparts_local>0){
          float valmin_local, valmax_local;
          int ii;

          valmin_local =  1000000000.0;
          valmax_local = -1000000000.0;
          for(ii = 0; ii<nparts_local; ii++){
            valmin_local = MIN(valmin_local, rvals_local[ii]);
            valmax_local = MAX(valmax_local, rvals_local[ii]);
          }
          fprintf(stream_out_local, "%f %f\n", valmin_local, valmax_local);
        }
        else{
          fprintf(stream_out_local, "1.0 0.0\n");
        }
        rvals_local += nparts_local;
      }
    }
    CheckMemory;
  }
wrapup:
  FCLOSE_m(PART5FILE);
  fclose(stream_out_local);
  CheckMemory;
  FREEMEMORY(numtypes_local);
}

/* ------------------ CreatePartSizeFile ------------------------ */

void CreatePartSizeFile(partdata *parti){
  FILE *stream_local=NULL;
  LINT header_offset_local;

  if(parti->reg_file!=NULL)stream_local = fopen(parti->reg_file, "rb");
  if(parti->reg_file==NULL||stream_local==NULL)return;
  fclose(stream_local);
  header_offset_local =GetPartHeaderOffset(parti);
  stream_local = fopen(parti->bound_file, "r");
  if(stream_local==NULL){
    TestWrite(smokeview_scratchdir, &(parti->bound_file));
    CreatePartBoundFile(parti);
    stream_local = fopen(parti->bound_file, "r");
  }
  if(stream_local!=NULL){
    fclose(stream_local);
    TestWrite(smokeview_scratchdir, &(parti->size_file));
    CreatePartSizeFileFromBound(parti->bound_file, parti->size_file, header_offset_local);
    return;
  }
  printf("***warning: particle bound/size file %s could not be opened\n", parti->bound_file);
  printf("            particle sizing proceeding using the full particle file: %s\n", parti->reg_file);
  TestWrite(smokeview_scratchdir, &(parti->size_file));
  CreatePartSizeFileFromPart(parti->reg_file, parti->size_file, header_offset_local);
}

  /* ------------------ GetPartHistogramFile ------------------------ */
void GetPartHistogramFile(partdata *parti){
  int i;
  part5data *datacopy;

  if(parti->histograms == NULL){
    NewMemory((void **)&parti->histograms, npart5prop*sizeof(histogramdata *));
    for(i = 0; i < npart5prop; i++){
      NewMemory((void **)&parti->histograms[i], sizeof(histogramdata));
      InitHistogram(parti->histograms[i], NHIST_BUCKETS, NULL, NULL);
    }
  }
  for(i = 0; i<npart5prop; i++){
    ResetHistogram(parti->histograms[i], NULL, NULL);
  }
  datacopy = parti->data5;
  if(datacopy != NULL){
    for(i = 0; i < parti->ntimes; i++){
      int j;

      for(j = 0; j < parti->nclasses; j++){
        partclassdata *partclassi;
        float *rvals;
        int k;

        partclassi = parti->partclassptr[j];
        rvals = datacopy->rvals;

        if(rvals!=NULL){
          for(k = 2; k<partclassi->ntypes; k++){
            partpropdata *prop_id;
            int partprop_index;

            prop_id = GetPartProp(partclassi->labels[k].longlabel);
            if(prop_id==NULL)continue;

            partprop_index = prop_id-part5propinfo;
            UpdateHistogram(rvals, NULL,datacopy->npoints, parti->histograms[partprop_index]);
            rvals += datacopy->npoints;
          }
        }
        datacopy++;
      }
    }
  }
}

/* ------------------ MergePartHistograms ------------------------ */

void MergePartHistograms(void){
  int i;

  if(full_part_histogram==NULL){
    NewMemory((void **)&full_part_histogram, npart5prop*sizeof(histogramdata));
  }
  else{
    for(i=0;i<npart5prop;i++){
      FreeHistogram(full_part_histogram+i);
    }
  }
  for(i=0;i<npart5prop;i++){
    InitHistogram(full_part_histogram+i, NHIST_BUCKETS, NULL, NULL);
  }
  for(i = 0; i<npartinfo; i++){
    partdata *parti;
    int j;

    parti = partinfo + i;
    if(parti->loaded==0)continue;
    for(j = 0; j < parti->nclasses; j++){
      partclassdata *partclassi;
      int k;

      partclassi = parti->partclassptr[j];
      for(k = 2; k<partclassi->ntypes; k++){
        partpropdata *prop_id;
        int partprop_index;

        prop_id = GetPartProp(partclassi->labels[k].longlabel);
        if(prop_id==NULL)continue;

        partprop_index = prop_id-part5propinfo;
        MergeHistogram(full_part_histogram+partprop_index, parti->histograms[partprop_index], MERGE_BOUNDS);
      }
    }
  }
}

/* ------------------ GeneratePartHistograms ------------------------ */

void GeneratePartHistograms(void){
  int i;

#ifdef pp_HIST
  EnableDisablePartPercentileDraw(0);
#endif
  for(i=0;i<npartinfo;i++){
    partdata *parti;

    parti = partinfo + i;
    if(parti->loaded==1){
      GetPartHistogramFile(parti);
    }
  }
  MergePartHistograms();
#ifdef pp_HIST
  EnableDisablePartPercentileDraw(1);
#endif
  if(in_part_mt == 1){
    printf("particle setup complete\n");
    in_part_mt = 0;
  }
}

/* ------------------ GetPartData ------------------------ */

void GetPartData(partdata *parti, int nf_all_arg, FILE_SIZE *file_size_arg){
  FILE_m *PART5FILE;
  int class_index;
  int one_local, version_local, nclasses_local;
  int skip_local, nparts_local;
  int *numtypes_local = NULL, *numtypescopy_local, *numpoints_local = NULL;
  int numtypes_temp_local[2];
  int count_local, count2_local, first_frame_local = 1;
  size_t returncode;
  float time_local;
  part5data *datacopy_local;

  *file_size_arg = GetFileSizeSMV(parti->reg_file);

  if(parti->stream!=NULL){
    FCLOSE_m(parti->stream);
    parti->stream = NULL;
  }
  PART5FILE = fopen_m(parti->reg_file, "rbm");
  parti->stream = PART5FILE;
  if(PART5FILE==NULL)return;

  FSEEK_m(PART5FILE,4,SEEK_CUR);
  FREAD_m(&one_local,4,1,PART5FILE);
  FSEEK_m(PART5FILE,4,SEEK_CUR);

  FORTPART5READ_m(&version_local, 1);
  if(returncode==FAIL_m)goto wrapup;

  FORTPART5READ_m(&nclasses_local,1);
  if(returncode==FAIL_m)goto wrapup;

  NewMemory((void **)&numtypes_local,2*nclasses_local*sizeof(int));
  NewMemory((void **)&numpoints_local,nclasses_local*sizeof(int));
  numtypescopy_local =numtypes_local;
  numtypes_temp_local[0]=0;
  numtypes_temp_local[1]=0;
  CheckMemory;
  for(class_index=0;class_index<nclasses_local;class_index++){
    FORTPART5READ_m(numtypes_temp_local,2);
    if(returncode==FAIL_m)goto wrapup;
    *numtypescopy_local++=numtypes_temp_local[0];
    *numtypescopy_local++=numtypes_temp_local[1];
    skip_local = 2*(numtypes_temp_local[0]+numtypes_temp_local[1])*(8 + 30);
    FSEEK_m(PART5FILE,skip_local,SEEK_CUR);
    if(returncode==FAIL_m)goto wrapup;
  }
  CheckMemory;

  datacopy_local = parti->data5;
  count_local =0;
  count2_local =-1;
  parti->ntimes = 0;
  for(;;){
    int doit_local;

    CheckMemory;
    if(count_local>=nf_all_arg)break;
    FORTPART5READ_m(&time_local,1);
    if(returncode==FAIL_m)break;

    if((tload_step>1       && count_local%tload_step!=0)||
       (use_tload_begin==1 && time_local<tload_begin-TEPS)||
       (use_tload_end==1   && time_local>tload_end+TEPS)){
      doit_local=0;
    }
    else{
      count2_local++;
      doit_local =1;
    }
    count_local++;

    if(doit_local==1){
      parti->times[count2_local]=time_local;
      parti->ntimes = count2_local+1;
    }
    for(class_index=0;class_index<nclasses_local;class_index++){
      FORTPART5READ_m(&nparts_local,1);
      if(returncode==FAIL_m)goto wrapup;
      numpoints_local[class_index] = nparts_local;
      skip_local=0;
      CheckMemory;
      if(doit_local==1){
        float *xyz;
        int j;

        FORTPART5READ_mv((void **)&xyz, NXYZ_COMP_PART*nparts_local);
        if(returncode==FAIL_m)goto wrapup;
        CheckMemory;
        if(nparts_local>0){
          float *x_local, *y_local, *z_local;
          short *sx_local, *sy_local, *sz_local;

          x_local = xyz;
          y_local = xyz+nparts_local;
          z_local = xyz+2*nparts_local;
          sx_local = datacopy_local->sx;
          sy_local = datacopy_local->sy;
          sz_local = datacopy_local->sz;
          for(j=0;j<nparts_local;j++){
            float xx_local, yy_local, zz_local;
            int factor_local=256*128-1;

            xx_local = FDS2SMV_X(x_local[j])/xbar;
            yy_local = FDS2SMV_Y(y_local[j])/ybar;
            zz_local = FDS2SMV_Z(z_local[j])/zbar;

            sx_local[j] = factor_local*xx_local;
            sy_local[j] = factor_local*yy_local;
            sz_local[j] = factor_local*zz_local;
          }
          CheckMemory;
        }
      }
      else{
        skip_local += 4+NXYZ_COMP_PART*nparts_local*sizeof(float)+4;
      }
      CheckMemory;
      if(doit_local==1){
        int *sort_tags_local;
        int j;

        sort_tags_local = datacopy_local->sort_tags;
        FORTPART5READ_m(datacopy_local->tags,nparts_local);
        if(returncode==FAIL_m)goto wrapup;
        CheckMemory;
        if(nparts_local>0){
          for(j=0;j<nparts_local;j++){
            sort_tags_local[2*j]=datacopy_local->tags[j];
            sort_tags_local[2*j+1]=j;
          }
          qsort( sort_tags_local, (size_t)nparts_local, 2*sizeof(int), CompareTags);
        }
      }
      else{
        skip_local += 4 + nparts_local*sizeof(int) + 4;  // skip over tag
      }
      CheckMemory;
      if(doit_local==1){
        int part_type;
        if(numtypes_local[2*class_index]>0){
          float *valmin_smv, *valmax_smv;

          FORTPART5READ_mv((void **)&(datacopy_local->rvals), nparts_local*numtypes_local[2*class_index]);
          if(returncode==FAIL_m)goto wrapup;

          valmin_smv = parti->valmin_smv;
          valmax_smv = parti->valmax_smv;
          for(part_type = 0; part_type<numtypes_local[2*class_index]; part_type++){
            int prop_index, k;
            float *vals;

            prop_index = GetPartPropIndex(class_index, part_type+2);
            vals = datacopy_local->rvals+part_type*nparts_local;
            for(k = 0; k<nparts_local; k++){
              float val;

              val = *vals++;
              if(valmin_smv[prop_index]>valmax_smv[prop_index]){
                valmin_smv[prop_index] = val;
                valmax_smv[prop_index] = val;
              }
              else{
                valmin_smv[prop_index] = MIN(val, valmin_smv[prop_index]);
                valmax_smv[prop_index] = MAX(val, valmax_smv[prop_index]);
              }
            }
          }
          if(returncode==FAIL_m)goto wrapup;
        }
      }
      else{
        if(numtypes_local[2*class_index]>0){
          skip_local += 4 + nparts_local*numtypes_local[2*class_index]*sizeof(float) + 4;  // skip over vals for now
        }
      }
      CheckMemory;

      if(skip_local>0){
        FSEEK_m(PART5FILE,skip_local,SEEK_CUR);
        if(returncode==FAIL_m)goto wrapup;
      }
      else{
        datacopy_local++;
      }
      CheckMemory;
    }
    CheckMemory;
    if(first_frame_local==1)first_frame_local =0;
  }
wrapup:
  UpdateAllPartVis(parti);
  CheckMemory;
  FREEMEMORY(numtypes_local);
  FREEMEMORY(numpoints_local);
}

/* ------------------ PrintPartProp ------------------------ */

#ifdef _DEBUG
void PrintPartProp(void){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(STRCMP(propi->label->longlabel, "Uniform color")==0){
      PRINTF("label=%s\n", propi->label->longlabel);
    }
    else{
      PRINTF("label=%s min=%f max=%f\n", propi->label->longlabel, propi->valmin, propi->valmax);
      PRINTF("   glbmin=%f glbmax=%f\n", propi->dlg_global_valmin, propi->dlg_global_valmax);
#ifdef pp_HIST
      PRINTF("   permin=%f permax=%f\n", propi->percentile_min, propi->percentile_max);
#endif
    }
    PRINTF("\n");
  }
}
#endif


/* ------------------ GetPartPropIndexS ------------------------ */

int GetPartPropIndexS(char *shortlabel){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->shortlabel,shortlabel)==0)return i;
  }
  return -1;
}

/* ------------------ GetPartPropIndex ------------------------ */

int GetPartPropIndex(int class_i, int class_i_j){
  int i;
  char *label;
  partclassdata *partclassi;
  flowlabels *labels, *labelj;

  assert(class_i>=0&&class_i<npartclassinfo);
  class_i = CLAMP(class_i,0, npartclassinfo-1);
  partclassi = partclassinfo+class_i;

  labels = partclassi->labels;
  assert(class_i_j>=0&&class_i_j<partclassi->ntypes);
  class_i_j = CLAMP(class_i_j,0, partclassi->ntypes-1);
  labelj = labels+class_i_j;

  label = labelj->longlabel;
  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(label!=NULL&&strcmp(propi->label->longlabel,label)==0)return i;
  }
  return 0;
}

/* ------------------ GetPartProp ------------------------ */

partpropdata *GetPartProp(char *label){
  int i;

  for(i=0;i<npart5prop;i++){
    partpropdata *propi;

    propi = part5propinfo + i;
    if(strcmp(propi->label->longlabel,label)==0)return propi;
  }
  return NULL;
}

/* ------------------ InitPartProp ------------------------ */

void InitPartProp(void){
  int i,j,k;

  // 0.  only needed if InitPartProp is called more than once
  // (and if so, need to also free memory of each component)

  FREEMEMORY(part5propinfo);
  npart5prop=0;

  // 1.  count max number of distinct variables

  for(i=0;i<npartclassinfo;i++){
    partclassdata *partclassi;

    partclassi = partclassinfo + i;
    npart5prop+=(partclassi->ntypes-1);  // don't include first type which is hidden
  }

  // 2. now count the exact amount and put labels into array just allocated

  if(npart5prop>0){
    NewMemory((void **)&part5propinfo,npart5prop*sizeof(partpropdata));
    npart5prop=0;

    for(i=0;i<npartclassinfo;i++){
      partclassdata *partclassi;

      partclassi = partclassinfo + i;
      for(j=1;j<partclassi->ntypes;j++){ // skip over first type which is hidden
        flowlabels *flowlabel;
        int define_it;

        define_it = 1;
        flowlabel = partclassi->labels + j;
        for(k=0;k<npart5prop;k++){
          partpropdata *propi;
          char *proplabel;

          propi = part5propinfo + k;
          proplabel = propi->label->longlabel;
          if(strcmp(proplabel,flowlabel->longlabel)==0){
            define_it=0;
            break;
          }
        }
        if(define_it==1){
          partpropdata *propi;

          propi = part5propinfo + npart5prop;

          propi->label=flowlabel;
          propi->setvalmin=GLOBAL_MIN;
          propi->setvalmax=GLOBAL_MAX;
          propi->set_global_bounds=1;
          propi->dlg_global_valmin=100000000.0;
          propi->dlg_global_valmax=-propi->dlg_global_valmin;
          propi->valmin=1.0;
          propi->valmax=0.0;
#ifdef pp_HIST
          propi->percentile_min=1.0;
          propi->percentile_max=0.0;
#endif
          propi->user_min=1.0;
          propi->user_max=0.0;
          propi->display=0;


          propi->setchopmin=0;
          propi->setchopmax=0;
          propi->chopmin=1.0;
          propi->chopmax=0.0;

          propi->partlabelvals = NULL;
          NewMemory((void **)&propi->partlabelvals, 256*sizeof(float));
#ifdef pp_HIST
          propi->buckets = NULL;
          InitHistogram(&propi->histogram, NHIST_BUCKETS, NULL, NULL);
#endif
          npart5prop++;
        }
      }

    }
  }
  for(i=0;i<npart5prop;i++){
    partpropdata *propi;
    int ii;

    propi = part5propinfo + i;

    propi->class_present=NULL;
    propi->class_vis=NULL;
    propi->class_types=NULL;
    NewMemory((void **)&propi->class_types,npartclassinfo*sizeof(unsigned int));
    NewMemory((void **)&propi->class_present,npartclassinfo*sizeof(unsigned char));
    NewMemory((void **)&propi->class_vis,npartclassinfo*sizeof(unsigned char));
    for(ii=0;ii<npartclassinfo;ii++){
      propi->class_vis[ii]=1;
      propi->class_present[ii]=0;
      propi->class_types[ii]=0;
    }
  }
  for(i=0;i<npartclassinfo;i++){
    partclassdata *partclassi;

    partclassi = partclassinfo + i;
    for(j=1;j<partclassi->ntypes;j++){
      flowlabels *flowlabel;
      partpropdata *classprop;

      flowlabel = partclassi->labels + j;
      classprop = GetPartProp(flowlabel->longlabel);
      if(classprop!=NULL){
        classprop->class_present[i]=1;
        classprop->class_types[i]=j-2;
      }
    }
  }
}

/* ------------------ GetNPartFrames ------------------------ */

int GetNPartFrames(partdata *parti){
  FILE *stream;
  char buffer[256];
  float time_local;
  char *reg_file, *size_file, *bound_file;
  int i;
  int doit = 0;
  int stat_sizefile, stat_regfile;
  STRUCTSTAT stat_sizefile_buffer, stat_regfile_buffer;
  int nframes_all;

  reg_file=parti->reg_file;
  size_file=parti->size_file;
  bound_file = parti->bound_file;

  // if size file doesn't exist then generate it

  stat_sizefile=STAT(size_file,&stat_sizefile_buffer);
  stat_regfile=STAT(reg_file,&stat_regfile_buffer);
  if(stat_regfile!=0)return -1;

  // create a size file if 1) the size does not exist
  //                       2) base file is newer than the size file
  // create_part5sizefile(reg_file,size_file);

  if(stat_regfile_buffer.st_size > parti->reg_file_size){                   // particle file has grown
    parti->reg_file_size = stat_regfile_buffer.st_size;
    doit = 1;
  }
  if(doit==1||stat_sizefile != 0 || stat_regfile_buffer.st_mtime>stat_sizefile_buffer.st_mtime){

    TrimBack(reg_file);
    TrimBack(size_file);
    TrimBack(bound_file);
    CreatePartSizeFile(parti);
  }

  stream=fopen(size_file,"r");
  if(stream==NULL)return -1;

  nframes_all=0;
  for(;;){
    int exitloop;

    if(fgets(buffer,255,stream)==NULL)break;
    sscanf(buffer,"%f",&time_local);
    exitloop=0;
    for(i=0;i<parti->nclasses;i++){
      if(fgets(buffer,255,stream)==NULL){
        exitloop=1;
        break;
      }
    }
    if(exitloop==1)break;
    nframes_all++;
  }
  fclose(stream);
  return nframes_all;
}

/* ------------------ GetMinPartFrames ------------------------ */

int GetMinPartFrames(int flag){
  int i;
  int min_frames=-1;

  for(i=0;i<npartinfo;i++){
    partdata *parti;
    int nframes;

    parti = partinfo + i;
    if(flag == PARTFILE_LOADALL ||
      (flag == PARTFILE_RELOADALL&&parti->loaded == 1) ||
      (flag >= 0 && i == flag)){

      nframes = GetNPartFrames(parti);
      if(nframes > 0){
        if(min_frames == -1){
          min_frames = nframes;
        }
        else{
          if(nframes != -1 && nframes < min_frames)min_frames = nframes;
        }
      }
    }
  }
  return min_frames;
}

#define FORCE 1
#define NOT_FORCE 0

/* ------------------ GetPartHeader ------------------------ */

int GetPartHeader(partdata *parti, int *nf_all, int option_arg, int print_option_arg){
  FILE *stream;
  char buffer_local[256];
  float time_local;
  int i;
  int count_local, nframes_all_local, sizefile_status_local;

  parti->ntimes=0;

  sizefile_status_local = GetSizeFileStatus(parti);
  if(sizefile_status_local== -1)return 0; // particle file does not exist so cannot be sized
  if(option_arg==FORCE||sizefile_status_local== 1){        // size file is missing or older than particle file
    TrimBack(parti->reg_file);
    TrimBack(parti->size_file);
    CreatePartSizeFile(parti);
  }

  stream=fopen(parti->size_file,"r");
  if(stream==NULL)return 0;

    // pass 1: count frames

  nframes_all_local =0;
  for(;;){
    int exitloop_local;

    if(fgets(buffer_local,255,stream)==NULL)break;
    sscanf(buffer_local,"%f",&time_local);
    exitloop_local =0;
    for(i=0;i<parti->nclasses;i++){
      if(fgets(buffer_local,255,stream)==NULL||(npartinfo>1&&npartframes_max!=-1&&nframes_all_local+1>npartframes_max)){
        exitloop_local =1;
        break;
      }
    }
    if(exitloop_local==1)break;
    nframes_all_local++;
    if(tload_step>1       && (nframes_all_local-1)%tload_step!=0)continue;
    if(use_tload_begin==1 && time_local<tload_begin-TEPS)continue;
    if(use_tload_end==1   && time_local>tload_end+TEPS)break;
    (parti->ntimes)++;
  }
  rewind(stream);
  *nf_all = nframes_all_local;
  if(parti->ntimes==0){
    fclose(stream);
    return 0;
  }

  // allocate memory for number of time steps * number of classes

  CheckMemory;
  NewMemory((void **)&parti->data5,   parti->nclasses*parti->ntimes*sizeof(part5data));
  NewMemory((void **)&parti->times,   parti->ntimes*sizeof(float));
  NewMemory((void **)&parti->filepos, nframes_all_local*sizeof(LINT));

  // free memory for x, y, z frame data

  for(i=0;i<parti->nclasses;i++){
    partclassdata *partclassi;

    partclassi = parti->partclassptr[i];
    partclassi->maxpoints=0;
  }

  // pass 2 - allocate memory for x, y, z frame data

  int nall_points_local;
  {
    part5data *datacopy_local;
    int fail_local;
    LINT filepos_local;
    int nall_points_types_local;

    fail_local =0;
    count_local =-1;
    datacopy_local =parti->data5;
    for(i=0;i<nframes_all_local;i++){
      int j;
      char format[128];
      int skipit;

      count_local++;
      fail_local =0;
      if(fgets(buffer_local,255,stream)==NULL){
        fail_local =1;
        break;
      }
      filepos_local = -1;
#ifdef INTEL_WIN_COMPILER
      strcpy(format, "%f %lli");
#else
      strcpy(format, "%f %li");
#endif
      sscanf(buffer_local, format, &time_local, &filepos_local);
      parti->filepos[count_local] = filepos_local;               // record file position for every frame
      skipit = 0;
      if(tload_step>1       && count_local%tload_step!=0)skipit = 1;
      if(use_tload_begin==1 && time_local<tload_begin-TEPS)skipit = 1;
      if(use_tload_end==1   && time_local>tload_end+TEPS)break;
      if(skipit == 1){
        for(j=0;j<parti->nclasses;j++){
          if(fgets(buffer_local,255,stream)==NULL){
            fail_local =1;
          }
        }
        if(fail_local==1)break;
        continue;
      }
      for(j=0;j<parti->nclasses;j++){
        int npoints_local;
        partclassdata *partclassj;

        datacopy_local->time = time_local;
        partclassj = parti->partclassptr[j];
        InitPart5Data(datacopy_local,partclassj);
        if(fgets(buffer_local,255,stream)==NULL){
          fail_local =1;
          break;
        }
        sscanf(buffer_local,"%i",&datacopy_local->npoints);
        npoints_local =datacopy_local->npoints;
        if(npoints_local>partclassj->maxpoints)partclassj->maxpoints=npoints_local;
        if(npoints_local>0){
          if(partfast==NO){
            NewMemory((void **)&datacopy_local->dsx, npoints_local*sizeof(float));
            NewMemory((void **)&datacopy_local->dsy, npoints_local*sizeof(float));
            NewMemory((void **)&datacopy_local->dsz, npoints_local*sizeof(float));
          }
        }
        datacopy_local++;
      }
      if(fail_local==1)break;
    }
    if(fail_local==1)parti->ntimes=i;
    fclose(stream);

    nall_points_types_local = 0;
    nall_points_local = 0;
    datacopy_local =parti->data5;
    for(i=0;i<parti->ntimes;i++){
      int j;

      for(j=0;j<parti->nclasses;j++){
        int npoints_local, ntypes_local;

        npoints_local            = datacopy_local->npoints;
        ntypes_local             = datacopy_local->partclassbase->ntypes;
        nall_points_types_local += npoints_local*ntypes_local;
        nall_points_local       += npoints_local;
        datacopy_local++;
      }
    }
    if(nall_points_local>0){
      FREEMEMORY(parti->vis_part);
      FREEMEMORY(parti->tags);
      FREEMEMORY(parti->sort_tags);
      FREEMEMORY(parti->sx);
      FREEMEMORY(parti->sy);
      FREEMEMORY(parti->sz);

      NewMemory((void **)&parti->vis_part,    nall_points_local*sizeof(unsigned char));
      NewMemory((void **)&parti->tags,        nall_points_local*sizeof(int));
      NewMemory((void **)&parti->sort_tags, 2*nall_points_local*sizeof(int));
      NewMemory((void **)&parti->sx,          nall_points_local*sizeof(short));
      NewMemory((void **)&parti->sy,          nall_points_local*sizeof(short));
      NewMemory((void **)&parti->sz,          nall_points_local*sizeof(short));
    }
    if(nall_points_types_local>0){
      FREEMEMORY(parti->irvals);
      NewMemory((void **)&parti->irvals, nall_points_types_local*sizeof(unsigned char));
    }

    datacopy_local =parti->data5;
    nall_points_types_local = 0;
    nall_points_local = 0;
    for(i=0;i<parti->ntimes;i++){
      int j;

      for(j=0;j<parti->nclasses;j++){
        int npoints_local, ntypes_local;

        datacopy_local->irvals    = parti->irvals    +     nall_points_types_local;
        datacopy_local->vis_part  = parti->vis_part  +     nall_points_local;
        datacopy_local->tags      = parti->tags      +     nall_points_local;
        datacopy_local->sort_tags = parti->sort_tags +   2*nall_points_local;
        datacopy_local->sx        = parti->sx        +     nall_points_local;
        datacopy_local->sy        = parti->sy        +     nall_points_local;
        datacopy_local->sz        = parti->sz        +     nall_points_local;

        npoints_local            = datacopy_local->npoints;
        ntypes_local             = datacopy_local->partclassbase->ntypes;
        nall_points_types_local += npoints_local*ntypes_local;
        nall_points_local       += npoints_local;
        datacopy_local++;
      }
    }
  }
  if(nall_points_local==0){
    return 0;
  }
  return 1;
}

/* ------------------ UpdatePartColors ------------------------ */

void UpdatePartColors(partdata *parti, int flag){
  int j;

  if(colorlabelpart==NULL){
    NewMemory((void **)&colorlabelpart, MAXRGB*sizeof(char *));
    {
      int n;

      for(n = 0; n<MAXRGB; n++){
        colorlabelpart[n] = NULL;
      }
      for(n = 0; n<nrgb; n++){
        NewMemory((void **)&colorlabelpart[n], 11);
      }
    }
  }
  if(parti!=NULL){
    if(parti->loaded==1&&parti->display==1){
      if(parti->stream!=NULL){
        GetPartColors(parti, nrgb, flag);
      }
      else{
        printf("***warning: particle data in %s was unloaded, colors not updated\n",parti->file);
      }
    }
  }
  else{
    for(j = 0; j<npartinfo; j++){
      partdata *partj;

      partj = partinfo+j;
      if(partj->loaded==1&&partj->display==1){
        if(partj->stream==NULL){
          printf("***warning: particle data in one or more particle files was unloaded, colors not updated\n");
          return;
        }
      }
    }
    for(j = 0; j<npartinfo; j++){
      partdata *partj;

      partj = partinfo+j;
      if(partj->loaded==1&&partj->display==1){
        GetPartColors(partj, nrgb, flag);
      }
    }
  }
}

/* -----  ------------- FinalizePartLoad ------------------------ */

void FinalizePartLoad(partdata *parti){
  int j;

  for(j = 0; j<npartinfo; j++){
    partdata *partj;

    partj = partinfo+j;
    if(partj->request_load==1){
      partj->request_load = 0;
      partj->loaded = 1;
      partj->display = 1;
    }
  }
  visParticles = 1;

  // generate histograms now rather than in the background if a script is running

#ifdef pp_HIST
  if(current_script_command!=NULL||part_multithread==0){
    GeneratePartHistograms();
  }
  else{
    update_generate_part_histograms = 1;
  }
#endif
  INIT_PRINT_TIMER(part_time1);
  GetGlobalPartBounds(ALL_FILES);
  PRINT_TIMER(part_time1, "particle get bounds time");
  if(cache_part_data==1){
    INIT_PRINT_TIMER(part_time2);
#ifdef pp_HIST
    SetPercentilePartBounds();
#endif
    for(j = 0; j<npartinfo; j++){
      partdata *partj;

      partj = partinfo+j;
      if(partj->loaded==1){
        UpdatePartColors(partj, 0);
      }
    }
    PRINT_TIMER(part_time2, "particle update colors time");
  }
#define BOUND_PERCENTILE_DRAW          120
  PartBoundsCPP_CB(BOUND_PERCENTILE_DRAW);
  parttype = 0;
  ParticlePropShowMenu(part5colorindex);
  plotstate = GetPlotState(DYNAMIC_PLOTS);
  UpdateTimes();
  UpdatePart5Extremes();
  updatemenu = 1;
  ForceIdle();
  glutPostRedisplay();
}

/* -----  ------------- ReadPart ------------------------ */

FILE_SIZE ReadPart(char *file_arg, int ifile_arg, int loadflag_arg, int *errorcode_arg){
  size_t lenfile_local;
  int error_local=0, nf_all_local;
  partdata *parti;
  FILE_SIZE file_size_local;
  float load_time_local;

#ifdef pp_PART_HIST
  if(loadflag_arg==UNLOAD&&part_multithread==1&&update_generate_part_histograms==-1){
    JOIN_PART_HIST;
  }
#endif
  SetTimeState();
  START_TIMER(load_time_local);
  assert(ifile_arg>=0&&ifile_arg<npartinfo);
  parti=partinfo+ifile_arg;

  FreeAllPart5Data(parti);

  if(parti->loaded==0&&loadflag_arg==UNLOAD)return 0.0;

  *errorcode_arg =0;
  parti->loaded = 0;
  parti->display=0;

  LOCK_PART_LOAD;
  plotstate=GetPlotState(DYNAMIC_PLOTS);
  updatemenu=1;
  UNLOCK_PART_LOAD;

  FREEMEMORY(parti->times);
  FREEMEMORY(parti->filepos);

  if(loadflag_arg==UNLOAD){
    if(parti->finalize == 1){
      UpdatePartColors(parti, 0);
      UpdateTimes();
      updatemenu = 1;
      UpdatePart5Extremes();
      PrintMemoryInfo;
    }
    return 0.0;
  }

  lenfile_local = strlen(file_arg);
  if(lenfile_local==0){
    ReadPart("",ifile_arg,UNLOAD,&error_local);
    UpdateTimes();
    return 0.0;
  }

  if(part_multithread==1){
    LOCK_PART_LOAD;
    PrintPartLoadSummary(PART_BEFORE, PART_LOADING);
    UNLOCK_PART_LOAD;
  }
  else{
    PRINTF("Loading %s", file_arg);
  }
  int have_particles;

  have_particles = GetPartHeader(parti, &nf_all_local, NOT_FORCE, 1);
  if(have_particles==0){
    ReadPart("", ifile_arg, UNLOAD, &error_local);
    return 0.0;
  }
  CheckMemory;
  GetPartData(parti, nf_all_local, &file_size_local);
  CheckMemory;
  LOCK_PART_LOAD;
  parti->loaded = 1;
  parti->display = 1;
#ifndef pp_HIST
  parti->hist_update=1;
#endif
  if(cache_part_data==0){
    UpdatePartColors(parti, 0);
  }
  UNLOCK_PART_LOAD;
  if(cache_part_data==0){
    FCLOSE_m(parti->stream);
    parti->stream = NULL;
  }

  PrintMemoryInfo;

  parti->request_load = 1;
  if(part_multithread==1){
    if(npartinfo>1){
      LOCK_PART_LOAD;
      PrintPartLoadSummary(PART_AFTER, PART_LOADING);
      UNLOCK_PART_LOAD;
    }
  }
  else{
    if(parti->finalize == 1){
      INIT_PRINT_TIMER(finalize_part);
      FinalizePartLoad(parti);
      PRINT_TIMER(finalize_part, "finalize particle time");
    }
    STOP_TIMER(load_time_local);
    if(file_size_local>1000000000){
      PRINTF(" - %.1f GB/%.1f s\n", (float)file_size_local/1000000000., load_time_local);
    }
    else if(file_size_local>1000000){
      PRINTF(" - %.1f MB/%.1f s\n", (float)file_size_local/1000000., load_time_local);
    }
    else{
      PRINTF(" - %.0f kB/%.1f s\n", (float)file_size_local/1000., load_time_local);
    }
    update_part_bounds = 1;
  }
  return file_size_local;
}

/* ------------------ UpdatePartMenuLabels ------------------------ */

void UpdatePartMenuLabels(void){
  int i;
  partdata *parti;
  char label[128];
  int lenlabel;

  if(npartinfo>0){
    FREEMEMORY(partorderindex);
    NewMemory((void **)&partorderindex,sizeof(int)*npartinfo);
    for(i=0;i<npartinfo;i++){
      partorderindex[i]=i;
    }
    qsort( (int *)partorderindex, (size_t)npartinfo, sizeof(int), ComparePart);

    for(i=0;i<npartinfo;i++){
      parti = partinfo + i;
      STRCPY(parti->menulabel,"");
      STRCAT(parti->menulabel, "particles");
      lenlabel=strlen(parti->menulabel);
      if(nmeshes>1){
        meshdata *partmesh;

        partmesh = meshinfo + parti->blocknumber;
        sprintf(label,"%s",partmesh->label);
        if(lenlabel>0)STRCAT(parti->menulabel,", ");
        STRCAT(parti->menulabel,label);
      }
      if(showfiles==1||lenlabel==0){
        if(nmeshes>1||lenlabel>0)STRCAT(parti->menulabel,", ");
        STRCAT(parti->menulabel,parti->file);
      }
    }
  }
}
