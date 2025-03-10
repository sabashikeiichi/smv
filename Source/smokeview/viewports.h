#ifndef VIEWPORTS_H_DEFINED
#define VIEWPORTS_H_DEFINED

void ViewportClip(int quad, GLint s_left, GLint s_down);
void ViewportInfo(int quad, GLint s_left, GLint s_down);
void ViewportTimebar(int quad, GLint s_left, GLint s_down);
void ViewportHrrPlot(int quad, GLint s_left, GLint s_down);
void ViewportSlicePlot(int quad, GLint s_left, GLint s_down);
void ViewportVerticalColorbar(int quad, GLint s_left, GLint s_down);
void ViewportTitle(int quad, GLint s_left, GLint s_down);
#ifdef pp_HIST
void ViewportHistogram(int quad, GLint s_left, GLint s_down);
#endif
void ViewportScene(int quad, int view_mode, GLint s_left, GLint s_down, screendata *screen);
#endif
