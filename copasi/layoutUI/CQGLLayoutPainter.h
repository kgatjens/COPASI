// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layoutUI/CQGLLayoutPainter.h,v $
//   $Revision: 1.1 $
//   $Name:  $
//   $Author: gauges $
//   $Date: 2010/03/10 12:33:51 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef CQGLLAYOUTPAINTER_H__
#define CQGLLAYOUTPAINTER_H__

#include <QGLWidget>

#include <set>

class CCopasiDataModel;
class CLGraphicalObject;
class CLayout;
class CLLayoutRenderer;
class CLPoint;
class CLRenderInformationBase;
class CQFontRenderer;
class QMouseEvent;
class QShowEvent;

class CQGLLayoutPainter : public QGLWidget
{

  Q_OBJECT        // must include this if you use Qt signals/slots

public:
  enum STATE
  {
    STATE_SELECTION,
    STATE_DRAG,
    STATE_NORMAL
  };

public:
  CQGLLayoutPainter(const QGLFormat& format, QWidget *parent = 0);
  ~CQGLLayoutPainter();

  /**
   * sets the the x and y offset of the viewport.
   */
  void setCurrentPosition(double x, double y);

  /**
   * sets the the x offset of the viewport.
   */
  void setCurrentPositionX(double x);

  /**
   * sets the the y offset of the viewport.
   */
  void setCurrentPositionY(double y);

  /**
   * Returns the current x offset of the viewport.
   */
  double getCurrentPositionX() const;

  /**
   * Returns the current y offset of the viewport.
   */
  double getCurrentPositionY() const;

  /**
   * Sets the zoom factor.
   */
  void setZoomFactor(double);

  /**
   * Returns the zoom factor.
   */
  double getZoomFactor() const;

  /**
   * Updates the viewport.
   */
  void update();

  void resetView();

  double minX() const;

  double minY() const;

  double maxX() const;

  double maxY() const;

  void update(const CCopasiDataModel* pDocument, CLayout* pLayout, const CLRenderInformationBase* pRenderInfo, const QString& baseDir);

  void change_style(const CLRenderInformationBase* pRenderInfo, bool defaultStyle = false);

  /**
   * Reverts the currently selected curve if there is one.
   * If nothing is selected or the single selected item is not a curve,
   * nothing is done.
   */
  void revertCurve();

  /**
   * Returns the width of the current drawing area.
   */
  unsigned int getViewportWidth() const;

  /**
   * Returns the height of the current viewport area.
   */
  unsigned int getViewportHeight() const;

  /**
   * Returns the width of the layout part that is currently displayed.
   */
  double getCurrentWidth() const;

  /**
   * Returns the width of the layout part that is currently displayed.
   */
  double getCurrentHeight() const;

  /**
   * Returns the currently selected items.
   */
  std::set<CLGraphicalObject*> getSelection();

  /**
   * Sets the selection to the items in the given set.
   */
  void setSelection(const std::set<CLGraphicalObject*>& selection);

  /**
   * Sets the aspect for the renderer.
   */
  void setAspect(double aspect);

  /**
   * Returns the aspect from the renderer.
   */
  double getAspect() const;

  GLubyte* export_bitmap(double x, double y, double width, double height, unsigned int imageWidth, unsigned int imageHeight, bool drawSelection);

protected:
  bool mInitialized; // whether the gl context has been initialized
  double mMinX; // minimal x value of the layout
  double mMinY; // minimal y value of the layout
  double mMaxX; // maximal x value of the layout
  double mMaxY; // maximal y value of the layout
  double mCurrentZoom ; // current zoom factor
  double mCurrentPositionX; // x offset of the current viewport
  double mCurrentPositionY; // y offset of the current viewport
  unsigned int mViewportWidth; // the width of the current drawing area
  unsigned int mViewportHeight; // the height of the current drawing area
  CLLayoutRenderer* mpRenderer;

  // stores the position where the mouse was pressed
  QPoint mMousePressPosition;

  // stores the position where the mouse currently is
  QPoint mMouseCurrentPosition;

  // stores the position where the mouse was at the last move event
  // this is important to determine how far objects have to be moved.
  QPoint mMouseLastPosition;

  /**
  * is set to the button(s) that is (are) pressed and to Qt::NoButton once it is
  * released.
  */
  Qt::MouseButton mMouseButton;

  /**
   * flag that determines if a drag timeout has occured.
   */
  bool mDragTimeout;

  /**
   * stores the current state of the scene.
   */
  STATE mState;

  /**
   * Stores a pointer to the basepoint if we are currently
   * dragging one.
   */
  CLPoint* mpDragPoint;

  static const int MARGIN;

  // for windows and unix/linux we need to define some function pointers
  // for OpenGL extension functions
#ifdef _WIN32
// framebuffer object functions
  typedef GLenum(APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSEXT)(GLenum target);
  typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSEXT)(GLsizei n, GLuint* framebuffers);
  typedef void (APIENTRY* PFNGLGENRENDERBUFFERSEXT)(GLsizei n, GLuint* renderbuffers);
  typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERSEXT)(GLsizei n, GLuint* framebuffers);
  typedef void (APIENTRY* PFNGLDELETERENDERBUFFERSEXT)(GLsizei n, GLuint* renderbuffers);
  typedef void (APIENTRY* PFNGLBINDFRAMEBUFFEREXT)(GLenum target, GLuint framebuffer);
  typedef void (APIENTRY* PFNGLBINDRENDERBUFFEREXT)(GLenum target, GLuint renderbuffer);
  typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEEXT)(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height);
  typedef void (APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFEREXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
// multisample functions
  typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXT)(GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height);
  typedef void (APIENTRY* PFNGLBLITFRAMEBUFFEREXT)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
      GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
      GLbitfield mask, GLenum filter);
#else
#ifndef __APPLE__  // if it is not an apple, we assume it is an X11 system
  typedef GLenum(*PFNGLCHECKFRAMEBUFFERSTATUSEXT)(GLenum target);
  typedef void (*PFNGLGENFRAMEBUFFERSEXT)(GLsizei n, GLuint* framebuffers);
  typedef void (*PFNGLGENRENDERBUFFERSEXT)(GLsizei n, GLuint* renderbuffers);
  typedef void (*PFNGLDELETEFRAMEBUFFERSEXT)(GLsizei n, GLuint* framebuffers);
  typedef void (*PFNGLDELETERENDERBUFFERSEXT)(GLsizei n, GLuint* renderbuffers);
  typedef void (*PFNGLBINDFRAMEBUFFEREXT)(GLenum target, GLuint framebuffer);
  typedef void (*PFNGLBINDRENDERBUFFEREXT)(GLenum target, GLuint renderbuffer);
  typedef void (*PFNGLRENDERBUFFERSTORAGEEXT)(GLenum target, GLenum internalFormat, GLsizei width, GLsizei height);
  typedef void (*PFNGLFRAMEBUFFERRENDERBUFFEREXT)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
// multisample functions
  typedef void (*PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXT)(GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height);
  typedef void (*PFNGLBLITFRAMEBUFFEREXT)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                          GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                          GLbitfield mask, GLenum filter);
#endif // __APPLE__
#endif // WIN32
#ifndef __APPLE__
  PFNGLCHECKFRAMEBUFFERSTATUSEXT glCheckFramebufferStatusEXTPtr;
  PFNGLGENFRAMEBUFFERSEXT glGenFramebuffersEXTPtr;
  PFNGLGENRENDERBUFFERSEXT glGenRenderbuffersEXTPtr;
  PFNGLDELETEFRAMEBUFFERSEXT glDeleteFramebuffersEXTPtr;
  PFNGLDELETERENDERBUFFERSEXT glDeleteRenderbuffersEXTPtr;
  PFNGLBINDFRAMEBUFFEREXT glBindFramebufferEXTPtr;
  PFNGLBINDRENDERBUFFEREXT glBindRenderbufferEXTPtr;
  PFNGLRENDERBUFFERSTORAGEEXT glRenderbufferStorageEXTPtr;
  PFNGLFRAMEBUFFERRENDERBUFFEREXT glFramebufferRenderbufferEXTPtr;
  PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXT glRenderbufferStorageMultisampleEXTPtr;
  PFNGLBLITFRAMEBUFFEREXT glBlitFramebufferEXTPtr;
#endif // __APPLE__

protected:

  void draw();

  virtual void initializeGL();

  virtual void resizeGL(int w, int h);

  virtual void paintGL();

  virtual void mousePressEvent(QMouseEvent* pMouseEvent);

  virtual void mouseReleaseEvent(QMouseEvent* pMouseEvent);

  virtual void mouseMoveEvent(QMouseEvent* pMouseEvent);

  /**
   * This method sets the correct cursor and updates the status message for the
   * status bar.
   */
  void update_status_and_cursor();

  /**
   * Destroys the passed in render and framebuffers.
   */
  void destroy_buffers(GLuint& fbo, GLuint* rbuffers, GLuint& multiFBO, GLuint* multiRBuffers);

  /**
   * Checks the state of the currently bound framebuffer.
   * If the framebuffer is valid, true is returned, if it is invalid,
   * the return value is false and messageHeader and message contain a message
   * header and a message to display to the user.
   */
  bool check_fbo_status(QString& messageHeader, QString& message);

  /**
   * Renders the specified part of the model into a bitmap.
   * The bitmap is returned as RGBA data in pImageData.
   * If pImageData is pointing to a null pointer, memory is alocated and the caller has to release it.
   * If drawing the bitmap is successfull, true is returned, else false is
   * returned.
   */
  bool draw_bitmap(double x, double y, double width, double height,
                   unsigned int imageWidth, unsigned int imageHeight,
                   GLuint& fbo, GLuint& multiFBO,
                   GLuint** rbuffers, GLuint** multiRBuffers,
                   GLubyte** pImageData, GLuint samples = 0);

#ifndef __APPLE__
  /**
   * On non apple systems, we need to get the pointers to extension functions.
   */
  void initialize_extension_functions();

  /**
   * Set all pointer to extension function to NULL.
   */
  void clear_extension_functions();

#endif // __APPLE__

protected slots:
  /**
   * This slot is called if a timer runs down that is started on a left mouse press
   */
  void timeout();

signals:
  /**
   * signal is emitted when the status bar is supposed to show a certain
   * message.
   * The message text and the timeout in milliseconds are the arguments
   * to the signal.
   */
  void status_message(const QString& message, int timeout);

  /**
   * This signal is emitted when the painter changes
   * the document / layout.
   */
  void documentChanged();

  /**
   * This signal is emitted whenever the selection changes.
   * If after the change exactly one curve object is selected, the
   * argument to the signal is true, otherwise it is false
   */
  void singleCurveSelected(bool);
};

#endif /* CQGLLAYOUTPAINTER_H__ */