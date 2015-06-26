/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtWebEngineWidgets>
#include <QtWebChannel/QtWebChannel>
#include <QFile>
#include "mainwindow.h"

// clingo
#include "clingocontrol.hh"
#include <iostream>
using namespace std;
// end clingo

template<typename Arg, typename R, typename C>
struct InvokeWrapper {
    R *receiver;
    void (C::*memberFun)(Arg);
    void operator()(Arg result) {
        (receiver->*memberFun)(result);
    }
};

template<typename Arg, typename R, typename C>
InvokeWrapper<Arg, R, C> invoke(R *receiver, void (C::*memberFun)(Arg))
{
    InvokeWrapper<Arg, R, C> wrapper = {receiver, memberFun};
    return wrapper;
}

//! [1]

MainWindow::MainWindow(const QUrl& url, int argc, char **argv)
{
    progress = 0;

    saved_argc = argc;
    saved_argv = argv;

    QFile file;
    file.setFileName(":/jquery.min.js");
    file.open(QIODevice::ReadOnly);
    jQuery = file.readAll();
    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
    file.close();
//! [1]

//! [2]
    view = new QWebEngineView(this);


    QWebChannel* chan = new QWebChannel(); // TODO test, does it need qwebengineview or something in? see QQmlWebChannel
    view->page()->setWebChannel(chan);


    view->load(url);
    //connect(view, SIGNAL(loadFinished(bool)), SLOT(adjustLocation()));
    connect(view, SIGNAL(titleChanged(QString)), SLOT(adjustTitle()));
    connect(view, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(view, SIGNAL(loadFinished(bool)), SLOT(finishLoading(bool)));

    //locationEdit = new QLineEdit(this);
    //locationEdit->setSizePolicy(QSizePolicy::Expanding, locationEdit->sizePolicy().verticalPolicy());
    //connect(locationEdit, SIGNAL(returnPressed()), SLOT(changeLocation()));

    /*
    QToolBar *toolBar = addToolBar(tr("Navigation"));
    toolBar->addAction(view->pageAction(QWebEnginePage::Back));
    toolBar->addAction(view->pageAction(QWebEnginePage::Forward));
    toolBar->addAction(view->pageAction(QWebEnginePage::Reload));
    toolBar->addAction(view->pageAction(QWebEnginePage::Stop));
    toolBar->addWidget(locationEdit);
    */
//! [2]

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    QAction* viewSourceAction = new QAction("Page Source", this);
    connect(viewSourceAction, SIGNAL(triggered()), SLOT(viewSource()));
    viewMenu->addAction(viewSourceAction);

    // clingo
    QAction* runASPAction = new QAction("Run ASP", this);
    connect(runASPAction, SIGNAL(triggered()), SLOT(runASP()));
    viewMenu->addAction(runASPAction);
    // end clingo

//! [3]
    QMenu *effectMenu = menuBar()->addMenu(tr("&Effect"));
    effectMenu->addAction("Highlight all links", this, SLOT(highlightAllLinks()));

    rotateAction = new QAction(this);
    rotateAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    rotateAction->setCheckable(true);
    rotateAction->setText(tr("Turn images upside down"));
    connect(rotateAction, SIGNAL(toggled(bool)), this, SLOT(rotateImages(bool)));
    effectMenu->addAction(rotateAction);

    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(tr("Remove GIF images"), this, SLOT(removeGifImages()));
    toolsMenu->addAction(tr("Remove all inline frames"), this, SLOT(removeInlineFrames()));
    toolsMenu->addAction(tr("Remove all object elements"), this, SLOT(removeObjectElements()));
    toolsMenu->addAction(tr("Remove all embedded elements"), this, SLOT(removeEmbeddedElements()));

    setCentralWidget(view);
}
//! [3]

void MainWindow::runASP()
{
    QTextEdit* textEdit = new QTextEdit(NULL);
    textEdit->setAttribute(Qt::WA_DeleteOnClose);
    textEdit->adjustSize();
    textEdit->move(this->geometry().center() - textEdit->rect().center());
    textEdit->show();


    // clingo test
    DefaultGringoModule module;
    std::unique_ptr<ClingoLib> ctl;

    // Initialize control
    // TODO HOX NOTE: const_cast<const char**>(argv)) is very very suspect.
    //   Should check that this is OK... Actually should just fix ClingoLib
    //   constructor.
    ctl = Gringo::make_unique<ClingoLib>(module, saved_argc, const_cast<const char**>(saved_argv));


    QFile aspFile("testprog.asp");
    ctl->load(aspFile.fileName().toStdString());

    // About GroundVec:
    //   In clingo #scripts ground vector is the vector passed to ground command
    //   e.g. in prg.ground([("base", [])]), the Python list [("base", [])]
    //   is the GroundVec.
    //   C++ equivalent is to init a "Gringo::Control::GroundVec parts" and then
    //   use "parts.emplace_back("base", Gringo::FWValVec{});".
    //   At least "base" must be added, otherwise nothing is grounded and solver
    //   returns empty.
    Gringo::Control::GroundVec parts;
    parts.emplace_back("base", Gringo::FWValVec{});

    // Gringo::Any def in gringo/control.hh, some kind of 'Holder', data struct
    ctl->ground(parts, Gringo::Any());

    // About SolveResult:
    //   Gringo::SolveResult defined in gringo/control.hh:
    //     enum class SolveResult { UNKNOWN=0, SAT=1, UNSAT=2 };
    //
    //   ClingoControl::solveIter and ::solveAsync return Gringo::SolveIter and
    //   ::SolveFuture respectively. Use get() on these to get the SolveResult.
    Gringo::SolveResult ret;

    // About Assumptions:
    //   Gringo::Assumptions defined in gringo/control.hh:
    //     using Assumptions = std::vector<std::pair<Value, bool>>
    //
    //   Assumptions can be set by "ass.emplace_back(atom, truth);"
    //   NOTE: clasp has Solver::clearAssumptions()
    Gringo::Control::Assumptions ass;

    // About Modelhandler:
    //   Set to Gringo::Control::ModelHandler(nullptr) when no onModel handler.
    //   Otherwise onModel handler can be a lambda:
    //     [ /* capture */ ](Gringo::Model const &m) -> bool { /* do something with model m */ }
    //   Apparently ModelHandler should return true, TODO: determine what
    //   happens if false is returned.
    auto handler = [](Gringo::Model const &m) -> bool {
        cout << "handler called" << endl;
        Gringo::ValVec tmp = m.atoms(Gringo::Model::ATOMS);
        cout << "vec size: " << tmp.size() << endl;
        for(std::vector<int>::size_type i = 0; i != tmp.size(); i++) {
            /* std::cout << someVector[i]; ... */
            cout << "atom found:" << endl;
            cout << tmp[i] << endl;
        }
        if (m.contains(Gringo::Value::createId("a"))) {
            cout << "found 'a'" << endl;
        }
        else {
            cout << "did not find 'a'" << endl;
        }
        return true;
    };

    // ClingoControl::solve(omh, ass) takes onModel handler and assumptions
    ret = ctl->solve(Gringo::Control::ModelHandler(handler), std::move(ass));
    //ret = ctl->solve(Gringo::Control::ModelHandler(nullptr), std::move(ass));

    switch (ret) {
    case Gringo::SolveResult::SAT:      {
        cout << "SAT" << endl;
        textEdit->setPlainText("SAT");
        break;
    }
    case Gringo::SolveResult::UNSAT:    {
        cout << "UNSAT" << endl;
        textEdit->setPlainText("UNSAT");
        break;
    }
    case Gringo::SolveResult::UNKNOWN:  {
        cout << "UNKNOWN" << endl;
        textEdit->setPlainText("UNKNOWN");
        break;
    }
    default:                            {
        cout << "PROBLEM" << endl;
        textEdit->setPlainText("PROBLEM");
        break;
    }
    }
    // end clingo
}

void MainWindow::viewSource()
{
    QTextEdit* textEdit = new QTextEdit(NULL);
    textEdit->setAttribute(Qt::WA_DeleteOnClose);
    textEdit->adjustSize();
    textEdit->move(this->geometry().center() - textEdit->rect().center());
    textEdit->show();

    view->page()->toHtml(invoke(textEdit, &QTextEdit::setPlainText));
}

//! [4]
void MainWindow::adjustLocation()
{
    locationEdit->setText(view->url().toString());
}

void MainWindow::changeLocation()
{
    QUrl url = QUrl::fromUserInput(locationEdit->text());
    view->load(url);
    view->setFocus();
}
//! [4]

//! [5]
void MainWindow::adjustTitle()
{
    if (progress <= 0 || progress >= 100)
        setWindowTitle(view->title());
    else
        setWindowTitle(QString("%1 (%2%)").arg(view->title()).arg(progress));
}

void MainWindow::setProgress(int p)
{
    progress = p;
    adjustTitle();
}
//! [5]

//! [6]
void MainWindow::finishLoading(bool)
{
    progress = 100;
    adjustTitle();
    view->page()->runJavaScript(jQuery);

    rotateImages(rotateAction->isChecked());
}
//! [6]

//! [7]
void MainWindow::highlightAllLinks()
{
    // We append '; undefined' after the jQuery call here to prevent a possible recursion loop and crash caused by
    // the way the elements returned by the each iterator elements reference each other, which causes problems upon
    // converting them to QVariants.
    QString code = "qt.jQuery('a').each( function () { qt.jQuery(this).css('background-color', 'yellow') } ); undefined";
    view->page()->runJavaScript(code);
}
//! [7]

//! [8]
void MainWindow::rotateImages(bool invert)
{
    QString code;

    // We append '; undefined' after each of the jQuery calls here to prevent a possible recursion loop and crash caused by
    // the way the elements returned by the each iterator elements reference each other, which causes problems upon
    // converting them to QVariants.
    if (invert)
        code = "qt.jQuery('img').each( function () { qt.jQuery(this).css('-webkit-transition', '-webkit-transform 2s'); qt.jQuery(this).css('-webkit-transform', 'rotate(180deg)') } ); undefined";
    else
        code = "qt.jQuery('img').each( function () { qt.jQuery(this).css('-webkit-transition', '-webkit-transform 2s'); qt.jQuery(this).css('-webkit-transform', 'rotate(0deg)') } ); undefined";
    view->page()->runJavaScript(code);
}
//! [8]

//! [9]
void MainWindow::removeGifImages()
{
    QString code = "qt.jQuery('[src*=gif]').remove()";
    view->page()->runJavaScript(code);
}

void MainWindow::removeInlineFrames()
{
    QString code = "qt.jQuery('iframe').remove()";
    view->page()->runJavaScript(code);
}

void MainWindow::removeObjectElements()
{
    QString code = "qt.jQuery('object').remove()";
    view->page()->runJavaScript(code);
}

void MainWindow::removeEmbeddedElements()
{
    QString code = "qt.jQuery('embed').remove()";
    view->page()->runJavaScript(code);
}
//! [9]

