// Copyright (c) 2025 Kirill Gavrilov
// This file is based on original code licensed under the MIT License.
#pragma once

#ifdef _WIN32
  // should be included before other headers to avoid missing definitions
#include <windows.h>
#endif

#include "OcctQWidgetViewer.h"

#include "OcctQtTools.h"

#include <Standard_WarningsDisable.hxx>
#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <Standard_WarningsRestore.hxx>

#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_NeutralWindow.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Message.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS.hxx>

OcctQWidgetViewer::OcctQWidgetViewer(QWidget* theParent)
    : QWidget(theParent)
{
    Handle(Aspect_DisplayConnection) aDisp = new Aspect_DisplayConnection();
    Handle(OpenGl_GraphicDriver)     aDriver = new OpenGl_GraphicDriver(aDisp, false);

	this->SetLockOrbitZUp(true); //turn table orbit style

    // create viewer
    myViewer = new V3d_Viewer(aDriver);
    myViewer->SetDefaultBackgroundColor(Quantity_NOC_WHITE);
    myViewer->SetDefaultLights();

    // create AIS context
    myContext = new AIS_InteractiveContext(myViewer);

    // note - window will be created later within initializeGL() callback!
    myView = myViewer->CreateView();
    myView->SetImmediateUpdate(false);

#ifndef __APPLE__
    myView->ChangeRenderingParams().NbMsaaSamples = 0; // warning - affects performance
#endif

    myView->ChangeRenderingParams().ToShowStats = false;
    myView->ChangeRenderingParams().CollectedStats = (Graphic3d_RenderingParams::PerfCounters)(
        Graphic3d_RenderingParams::PerfCounters_FrameRate | Graphic3d_RenderingParams::PerfCounters_Triangles);

    // Qt widget setup
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow);   // request native window for this widget to create OpenGL context
    setMouseTracking(true);
    setBackgroundRole(QPalette::NoRole); // or NoBackground
    setFocusPolicy(Qt::StrongFocus);     // set focus policy to threat QContextMenuEvent from keyboard
    setUpdatesEnabled(true);

    initializeGL();
}

OcctQWidgetViewer::~OcctQWidgetViewer()
{
    Handle(Aspect_DisplayConnection) aDisp = myViewer->Driver()->GetDisplayConnection();

    // release OCCT viewer
    myContext->RemoveAll(false);
    myContext.Nullify();
    myView->Remove();
    myView.Nullify();
    myViewer.Nullify();

    aDisp.Nullify();
}

void OcctQWidgetViewer::dumpGlInfo(bool theIsBasic, bool theToPrint)
{
    TColStd_IndexedDataMapOfStringString aGlCapsDict;
    myView->DiagnosticInformation(aGlCapsDict,
        theIsBasic ? Graphic3d_DiagnosticInfo_Basic : Graphic3d_DiagnosticInfo_Complete);
    TCollection_AsciiString anInfo;
    for (TColStd_IndexedDataMapOfStringString::Iterator aValueIter(aGlCapsDict); aValueIter.More(); aValueIter.Next())
    {
        if (!aValueIter.Value().IsEmpty())
        {
            if (!anInfo.IsEmpty())
                anInfo += "\n";

            anInfo += aValueIter.Key() + ": " + aValueIter.Value();
        }
    }

    if (theToPrint)
        Message::SendInfo(anInfo);

    myGlInfo = QString::fromUtf8(anInfo.ToCString());
}

void OcctQWidgetViewer::initializeGL()
{
    const QRect           aRect = rect();
    const Graphic3d_Vec2i aViewSize(aRect.right() - aRect.left(), aRect.bottom() - aRect.top());
    const Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();

    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
    if (!aWindow.IsNull())
    {
        aWindow->SetNativeHandle(aNativeWin);
        aWindow->SetSize(aViewSize.x(), aViewSize.y());
        myView->SetWindow(aWindow);
        dumpGlInfo(true, true);
    }
    else
    {
        aWindow = new Aspect_NeutralWindow();
        aWindow->SetVirtual(true);
        aWindow->SetNativeHandle(aNativeWin);
        aWindow->SetSize(aViewSize.x(), aViewSize.y());
        myView->SetWindow(aWindow);
        dumpGlInfo(true, true);
    }
}

void OcctQWidgetViewer::displayAssembly(Assembly assembly) {

	myContext->RemoveAll(false);

    int parts = assembly.parts.size();

    for (int i = 0; i < parts; i++) {
        Handle(AIS_Shape) shape = new AIS_Shape(assembly.parts[i].Shape());
        shape->SetMaterial(Graphic3d_NOM_CHARCOAL);
        shape->SetColor(Quantity_Color(50, 1, 1, Quantity_TOC_HLS));
        myContext->Display(shape, AIS_Shaded, -1, false);

        TopTools_IndexedMapOfShape wireMap;
        TopExp::MapShapes(assembly.parts[i].Shape(), TopAbs_WIRE, wireMap);

        for (int j = 1; j <= wireMap.Extent(); ++j) {
            TopoDS_Wire wireTopo = TopoDS::Wire(wireMap(j));
            Handle(AIS_Shape) wire = new AIS_Shape(wireTopo);
            wire->SetColor(Quantity_Color(0.3,0.3,0.3, Quantity_TOC_RGB));
            //strok width?
            myContext->Display(wire, AIS_WireFrame, -1, false);
        }

        // joint mark
        for (int k = 0; k < assembly.parts[i].joints.size(); k++) {
            float length = 10;
            gp_Trsf joint = assembly.parts[i].joints[k];
            //gp_XYZ xyz = joint.TranslationPart();
            gp_XYZ xyz(0, 0, 0);
            gp_Quaternion q = joint.GetRotation();

            gp_Pnt x1(xyz);
            gp_Pnt x2(xyz.X() + length, xyz.Y(), xyz.Z());
            gp_Pnt y1(xyz);
            gp_Pnt y2(xyz.X(), xyz.Y()+length, xyz.Z());
            gp_Pnt z1(xyz);
            gp_Pnt z2(xyz.X(), xyz.Y(), xyz.Z()+length);

            TopoDS_Edge edgexTopo = BRepBuilderAPI_MakeEdge(x1, x2);
            edgexTopo = TopoDS::Edge(edgexTopo.Located(TopLoc_Location(joint)));
            Handle(AIS_Shape) edgex = new AIS_Shape(edgexTopo);
            edgex->SetColor(Quantity_NOC_RED);
            edgex->Attributes()->SetZLayer(Graphic3d_ZLayerId_Topmost);
            myContext->Display(edgex, AIS_WireFrame, -1, false);

            TopoDS_Edge edgeyTopo = BRepBuilderAPI_MakeEdge(y1, y2);
            edgeyTopo = TopoDS::Edge(edgeyTopo.Located(TopLoc_Location(joint)));
            Handle(AIS_Shape) edgey = new AIS_Shape(edgeyTopo);
            edgey->SetColor(Quantity_NOC_GREEN);
            edgey->Attributes()->SetZLayer(Graphic3d_ZLayerId_Topmost);
            myContext->Display(edgey, AIS_WireFrame, -1, false);

            TopoDS_Edge edgezTopo = BRepBuilderAPI_MakeEdge(z1, z2);
            edgezTopo = TopoDS::Edge(edgezTopo.Located(TopLoc_Location(joint)));
            Handle(AIS_Shape) edgez = new AIS_Shape(edgezTopo);
            edgez->SetColor(Quantity_NOC_BLUE);
            edgez->Attributes()->SetZLayer(Graphic3d_ZLayerId_Topmost);
            myContext->Display(edgez, AIS_WireFrame, -1, false);
        }

    }

    myView->Redraw();
}

void OcctQWidgetViewer::closeEvent(QCloseEvent* theEvent)
{
    theEvent->accept();
}

void OcctQWidgetViewer::keyPressEvent(QKeyEvent* theEvent)
{
}

void OcctQWidgetViewer::mousePressEvent(QMouseEvent* theEvent)
{
    QWidget::mousePressEvent(theEvent);
    if (myView.IsNull())
        return;

    const Graphic3d_Vec2i  aPnt(theEvent->pos().x(), theEvent->pos().y());
    const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
    if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
        updateView();
}

void OcctQWidgetViewer::mouseReleaseEvent(QMouseEvent* theEvent)
{
    QWidget::mouseReleaseEvent(theEvent);
    if (myView.IsNull())
        return;

    const Graphic3d_Vec2i  aPnt(theEvent->pos().x(), theEvent->pos().y());
    const Aspect_VKeyFlags aFlags = OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers());
    if (UpdateMouseButtons(aPnt, OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()), aFlags, false))
        updateView();
}

void OcctQWidgetViewer::mouseMoveEvent(QMouseEvent* theEvent)
{
    QWidget::mouseMoveEvent(theEvent);
    if (myView.IsNull())
        return;

    const Graphic3d_Vec2i aNewPos(theEvent->pos().x(), theEvent->pos().y());
    if (UpdateMousePosition(aNewPos,
        OcctQtTools::qtMouseButtons2VKeys(theEvent->buttons()),
        OcctQtTools::qtMouseModifiers2VKeys(theEvent->modifiers()),
        false))
    {
        updateView();
    }
}

void OcctQWidgetViewer::wheelEvent(QWheelEvent* theEvent)
{
    QWidget::wheelEvent(theEvent);
    if (myView.IsNull())
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    const Graphic3d_Vec2i aPos(Graphic3d_Vec2d(theEvent->position().x(), theEvent->position().y()));
#else
    const Graphic3d_Vec2i aPos(theEvent->pos().x(), theEvent->pos().y());
#endif
    if (!myView->Subviews().IsEmpty())
    {
        Handle(V3d_View) aPickedView = myView->PickSubview(aPos);
        if (!aPickedView.IsNull() && aPickedView != myFocusView)
        {
            // switch input focus to another subview
            OnSubviewChanged(myContext, myFocusView, aPickedView);
            updateView();
            return;
        }
    }

    if (UpdateZoom(Aspect_ScrollDelta(aPos, double(theEvent->angleDelta().y()) / 8.0)))
        updateView();
}

void OcctQWidgetViewer::updateView()
{
    update();
    // if (window() != NULL) { window()->update(); }
}

void OcctQWidgetViewer::paintEvent(QPaintEvent* theEvent)
{
    if (myView.IsNull() || myView->Window().IsNull())
        return;

    Aspect_Drawable aNativeWin = (Aspect_Drawable)winId();
    if (myView->Window()->NativeHandle() != aNativeWin)
    {
        // workaround window recreation done by Qt on monitor (QScreen) disconnection
        Message::SendWarning() << "Native window handle has changed by QWidget!";
        initializeGL();
        return;
    }

    const QRect     aRect = rect();
    Graphic3d_Vec2i aViewSizeNew(aRect.width(), aRect.height());
    Graphic3d_Vec2i aViewSizeOld;

    Handle(Aspect_NeutralWindow) aWindow = Handle(Aspect_NeutralWindow)::DownCast(myView->Window());
    aWindow->Size(aViewSizeOld.x(), aViewSizeOld.y());
    if (aViewSizeNew != aViewSizeOld)
    {
        aWindow->SetSize(aViewSizeNew.x(), aViewSizeNew.y());
        myView->MustBeResized();
        myView->Invalidate();
        dumpGlInfo(true, false);

        for (const Handle(V3d_View)& aSubviewIter : myView->Subviews())
        {
            aSubviewIter->MustBeResized();
            aSubviewIter->Invalidate();
        }
    }

    // flush pending input events and redraw the viewer
    Handle(V3d_View) aView = !myFocusView.IsNull() ? myFocusView : myView;
    aView->InvalidateImmediate();
    FlushViewEvents(myContext, aView, true);
}

void OcctQWidgetViewer::resizeEvent(QResizeEvent* theEvent)
{
    if (!myView.IsNull())
        myView->MustBeResized();
}

void OcctQWidgetViewer::handleViewRedraw(const Handle(AIS_InteractiveContext)& theCtx, const Handle(V3d_View)& theView)
{
    AIS_ViewController::handleViewRedraw(theCtx, theView);
    if (myToAskNextFrame)
        updateView(); // ask more frames for animation
}

void OcctQWidgetViewer::OnSubviewChanged(const Handle(AIS_InteractiveContext)&,
    const Handle(V3d_View)&,
    const Handle(V3d_View)& theNewView)
{
    myFocusView = theNewView;
}