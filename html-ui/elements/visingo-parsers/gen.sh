#!/bin/bash

pegjs -e visingo.parsers.answersettermsToVisjs answersetterms-to-visjs.pegjs
pegjs -e visingo.parsers.solverTxtToJson solver-txt-to-json.pegjs
