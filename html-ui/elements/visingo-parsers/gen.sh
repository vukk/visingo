#!/bin/bash

pegjs -e visingo.parsers.answersettermsToVisjs answersetterms-to-visjs.pegjs
pegjs -e visingo.parsers.solverTxtToJson solver-txt-to-json.pegjs
pegjs -e visingo.parsers.singleTerm single-term.pegjs
pegjs -e visingo.parsers.multipleTerm multiple-term.pegjs
