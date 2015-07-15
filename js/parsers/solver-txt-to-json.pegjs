/*
 * peg.js potassco solver default text to json parser
 * TODO: cleanup by using $str, add E* to all the things... (even after \n maybe)
 */

/* initializer */

{
  function makeInteger(o) {
    return parseInt(o.join(""), 10);
  }

  function extractList(list, index) {
    var result = [], i;

    for (i = 0; i < list.length; i++) {
      if (list[i][index] !== null) {
        result.push(list[i][index]);
      }
    }

    return result;
  }

  function buildList(first, rest, index) {
    return (first !== null ? [first] : []).concat(extractList(rest, index));
  }
}


// skip until we see line starting with clingo or clasp version
start
  = (!(("clasp" / "clingo") " version") anyline)*
    implemented:implemented
    anyline*
  { return implemented; }

anyline
  = [^\n]* "\n"

/* start, return json */

//start
implemented
  = sl:solververline il:inputline calls:calls result:resultline
    modelsmeta:modelsmeta numcalls:callsmeta timemeta:timemeta
  {
    var m = {
      Solver: sl,
      Input: il
    };
    // clingo will say there is one call if there are none TODO: fix if fixed upstream in clingo
    m['Call'] = (calls.length === 0) ? [{}] : calls;
    m['Result'] = result;
    m['Models'] = modelsmeta;
    m['Calls'] = numcalls;
    m['Time'] = timemeta;
    return m;
  }


/* solver version */

solververline
  = solver:("clingo" / "clasp") txt:" version " ver:[a-z0-9\._\-\(\)\t $:]i+ "\n"
  { return solver + txt + ver.join(''); }

/* input  */

// TODO: clasp and clingo should output all files used instead of ...
//       also filenames should be in quotes, otherwise files can't be recognized
//       without using stat on the filesystem, and we can't really do that from
//       the parser reliably
inputline
  = "Reading from " fp:filepath " ..."? "\n" { return [fp]; }


/* result */

resultline
  = result:result "\n\n" { return result; }

result
  = "SATISFIABLE" / "UNSATISFIABLE" / "UNKNOWN" / "OPTIMUM FOUND"


/* meta info */

modelsmeta
  = "Models" E* ":" E* num:posinteger more:"+"? E* "\n"
    opt:metaoptimumlinenl?
    costs:metaoptimizationlinenl?
  {
    var m = {
      Number: num,
      More: (more === null) ? "no" : "yes"
    };
    if(opt !== null) { m['Optimum'] = opt; m['Optimal'] = 1; } // TODO fix 'Optimal'
    if(opt !== null) m['Costs'] = costs;
    return m;
  }


metaoptimumlinenl "result optimum yes/no line"
  = E* "Optimum" E* ":" E* opt:("yes" / "no") "\n"
  { return opt; }

metaoptimizationlinenl "models result optimization line"
  = "Optimization" E* ":" E* first:integer rest:(" " integer)* "\n"
  { return buildList(first, rest, 1) }


callsmeta "final amount of calls"
  = "Calls" E* ":" E* num:posinteger "\n" { return num; }

timemeta
  = "Time" E* ":" E* total:timedecimal "s" E+ "(" E* "Solving:" E+ solving:timedecimal "s" E+
    "1st Model:" E+ model:timedecimal "s" E+ "Unsat:" E+ unsat:timedecimal "s" E* ")" "\n"
    "CPU Time" E* ":" E* cpu:timedecimal "s" "\n"?
  {
    return {
      Total: total,
      Solve: solving,
      Model: model,
      Unsat: unsat,
      CPU: cpu
    }
  }


/* multiple (solver) calls */

calls
  = call*

/* single (solver) call */

// "Solving..." then answer sets (models)
call
  = solvingtextnl models:(model*) { return { Witnesses: models }; }

solvingtextnl "Solving..."
  = "Solving...\n"

/* single model */

model
  = anl:answernumline "\n" pl:predicateline "\n" ol:optimizationlinenl?
  {
    var m = {};
    m['Value'] = pl;
    if(ol !== null) m['Costs'] = ol;
    return m;
  }

/* optimization line */

optimizationlinenl "answer optimization line"
  = ol:optimizationline "\n" { return ol; }

optimizationline
  = "Optimization: " first:integer rest:(" " integer)*
  { return buildList(first, rest, 1) }

/* answer number line */
answernumline "answer number line"
  = "Answer: " num:posinteger { return num; }
/* a line of predicates, separated by spaces */

predicateline
  = first:predicate rest:(S+ predicate)*
  { return buildList(first, rest, 1); }
  / E* { return []; }


/* a single predicate */

predicate "predicate"
  = $(ident "(" arguments ")")

// must have at least 1 argument (no empty predicates or tuples)
arguments
  = first:argument rest:("," S* argument)*
    { return buildList(first, rest, 1) }

argument
  = predicate
  / tag:ident
  / num:posinteger


/* numbers */

posinteger "positive integer"
  = digits:[0-9]+ { return makeInteger(digits); }

// digits:("-"? [0-9]+) doesn't work for some reason, pegjs bug?
integer "integer"
  = sign:"-"? digits:([0-9]+) { d = makeInteger(digits); return (sign === '-') ? -d : d }

timedecimal "positive decimal number"
  = float:$(characteristic:[0-9]+ "." decimal:[0-9]+)
  { return parseFloat(float); }


/* characters & strings */

// prefix allows default negation e.g. '-predicate(X,Y)'
ident "identifier"
  = $("-"? identStart identChar*)

// allow "_" on the start of idents, since heuristic predicates might be present in the output
identStart
  = [_a-z]i
  / nonascii

identChar
  = [_a-z0-9-]i
  / nonascii

nonascii
  = [\x80-\uFFFF]

E "space or tab"
  = [ \t\f]

S "whitespace"
  = [ \t\r\n\f]

/* file or other input */

filepath "file path"
  = $([A-Za-z0-9\-_\.\\\/]+)
