/*
 * ASP graph visualizer peg.js parser
 * TODO: don't break on whitespace
 */

/* initializer */

{
  // TODO: think about Object.observe
  var global = global || {nodes: {}, edges: {}};

  function makeInteger(o) {
    return parseInt(o.join(""), 10);
  }

  function extractList(list, index) {
    var result = [], i;

    for (i = 0; i < list.length; i++) {
      //if (list[i][index] !== null) {
        result.push(list[i][index]);
      //}
    }

    return result;
  }

  function buildList(first, rest, index) {
    return (first !== null ? [first] : []).concat(extractList(rest, index));
  }

  function ensure(key) {
    global[key] = global[key] || {};
  }

  function ensureEntity(entity, identity) {
    global[entity][identity] = global[entity][identity] || {id: identity};
  }

  function setDeep(objParam, arr, value) {
    obj = objParam;
    for (var i = 0, n = arr.length-1; i < n; ++i) {
        var key = arr[i];
        // simpler
        obj[key] = obj[key] || {};
        obj = obj[key];
        /*if (key in obj) {
            obj = obj[key];
        } else {
            return;
        }*/
    }
    obj[arr[arr.length-1]] = value;
    return obj;
  }

  function setAttr(obj, lst) {
    if(lst.length < 2) { return; } // need both path and value
    setDeep(obj, lst.slice(0, -1), lst[lst.length-1]);
  }
}

/* start, return json */
start
  = terms:terms "\r"? "\n"? { return global; }

terms
  = E* first:term? rest:(E+ term)* E*
  { return buildList(first, rest, 1); }

// predicate must be first (here we have specific predicates before...)
// booleanAtom must be before atom
// number must be before string (to support floats "1.2")
term // tuple is not allowed
  = entityNode
  / attrNode
  / entityEdge2
  / attrEdge2
  / entityEdge3
  / attrEdge3
  / attrGroup
  / attrNodeDefault
  / attrEdgeDefault
  / attrLayout
  / attrPhysics
  / attrInteraction
  / predicate
  / atom:booleanAtom
  / atom:predicateIdent
  / num:number
  / string:aspstring

entityNode
  = predicateName:"node" "(" id:entityIdent ")"
  { ensureEntity('nodes', id); }


// NOTE: if we want to change default later to not creating a node
//       we need to keep nodeAttrs as separate object, then copy
//       attrs for those nodes that do exist
attrNode
  = predicateName:"nodeAttr" "(" id:entityIdent "," rest:pathAndValue ")"
  {
    ensureEntity('nodes', id);
    if(rest.length < 2) { return; }
    setDeep(global.nodes[id], rest.slice(0, -1), rest[rest.length-1]);
  }

entityEdge2
  = predicateName:"edge" "(" node1:entityIdent "," node2:entityIdent ")"
  {
    ensureEntity('nodes', node1);
    ensureEntity('nodes', node2);
    var id = String(node1) + '%' + String(node2);
    ensureEntity('edges', id);
    global.edges[id].from = node1;
    global.edges[id].to = node2;
  }

entityEdge3
  = predicateName:"edge" "(" id:entityIdent "," node1:entityIdent "," node2:entityIdent ")"
  {
    ensureEntity('nodes', node1);
    ensureEntity('nodes', node2);
    ensureEntity('edges', id);
    global.edges[id].from = node1;
    global.edges[id].to = node2;
  }

attrGroup
  = predicateName:"groupAttr" "(" id:entityIdent "," rest:pathAndValue ")"
  {
    ensure('groups');
    global.groups[id] = global.groups[id] || {};
    setDeep(global.groups[id], rest.slice(0, -1), rest[rest.length-1]);
  }

attrEdge2
  = predicateName:"edgeAttr" "((" node1:entityIdent "," node2:entityIdent ")," rest:pathAndValue ")"
  {
    var id = String(node1) + '%' + String(node2);
    ensureEntity('edges', id);
    if(rest.length < 2) { return; }
    setDeep(global.edges[id], rest.slice(0, -1), rest[rest.length-1]);
  }

attrEdge3
  = predicateName:"edgeAttr" "(" id:entityIdent "," rest:pathAndValue ")"
  {
    ensureEntity('edges', id);
    //setAttr(global.edges[id], rest);
    if(rest.length < 2) { return; }
    setDeep(global.edges[id], rest.slice(0, -1), rest[rest.length-1]);
  }

attrNodeDefault
  = predicateName:"nodeDefault" "(" rest:pathAndValue ")"
  {
    if(rest.length < 2) { return; }
    ensure('nodeDefaults');
    setAttr(global.nodeDefaults, rest);
  }

attrEdgeDefault
  = predicateName:"edgeDefault" "(" rest:pathAndValue ")"
  {
    if(rest.length < 2) { return; }
    ensure('edgeDefaults');
    setAttr(global.edgeDefaults, rest);
  }

attrLayout
  = predicateName:"layout" "(" rest:pathAndValue ")"
  {
    if(rest.length < 2) { return; }
    ensure('layout');
    setAttr(global.layout, rest);
  }

attrPhysics
  = predicateName:"physics" "(" rest:pathAndValue ")"
  {
    if(rest.length < 2) { return; }
    ensure('physics');
    setAttr(global.physics, rest);
  }

attrInteraction
  = predicateName:"interaction" "(" rest:pathAndValue ")"
  {
    if(rest.length < 2) { return; }
    ensure('interaction');
    setAttr(global.interaction, rest);
  }



/* path and value */
pathAndValue
  = first:singlePathOrValue rest:("," singlePathOrValue)*
  {
    return buildList(first, rest, 1);
  }

// predicate must be first
// booleanAtom must be before atom
// number must be before string (to support floats "1.2")
// NOTE / TODO: path should contain only non-boolean atoms!
singlePathOrValue
  = atom:booleanAtom
  / atom:nullAtom
  / fun:insideFunction
  / atom:predicateIdent
  / num:number
  / string:aspstring
  / tuple:anontuple



/* a single predicate */

predicate
  = predicateName:predicateIdent "(" inside:arguments ")"
  { }

// inside a predicate, f() is a called a function
// parse any functions into strings for display purposes
insideFunction
  = $(predicateName:predicateIdent "(" inside:arguments ")")

arguments
  = first:argument rest:("," E* argument)*
  {
    return buildList(first, rest, 2);
  }

// predicate must be first
// booleanAtom must be before atom
// number must be before string (to support floats "1.2")
argument
  = predicate
  / atom:booleanAtom
  / atom:predicateIdent
  / tuple:anontuple
  / num:number
  / string:aspstring


/* tuples */

anontuple
  = "(" E* first:argument rest:("," E* argument)* E* ")"
  {
    return buildList(first, rest, 2);
  }

/* Booleans */
booleanAtom
  = "true" { return true; }
  / "false" { return false; }

/* Null, for resetting stuff */
nullAtom
  = "null" { return null; }

/* ASP strings */

aspstring "double quoted string"
  = "\"" str:string "\"" { return str; }

/* numbers */

posinteger "positive integer"
  = digits:[0-9]+ { return makeInteger(digits); }

integer "integer"
  = sign:"-"? digits:([0-9]+) { d = makeInteger(digits); return (sign === '-') ? -d : d }

decimal "decimal"
  = "\"" sign:"-"? float:$(characteristic:[0-9]+ "." decimal:[0-9]+) "\""
  {
    var f = parseFloat(float);
    return (sign === '-') ? -f : f;
  }

number "number"
  = decimal
  / integer

/* characters & strings */

// node, edge etc. identifiers
entityIdent "entity identifier"
  = num:integer
  / str:aspstring
  / atom:predicateIdent

// not " or newline related characters
string "string"
  = str:([^\"\r\n\f]+) { return str.join("") }

// prefix allows default negation e.g. '-predicate(X,Y)'
predicateIdent "predicate identifier"
  = prefix:$"-"? start:predicateIdentStart chars:predicateIdentChar* {
      return prefix + start + chars.join("");
    }

// allow "_" on the start of idents, since heuristic predicates might be present in the output
predicateIdentStart
  = [_a-z]i
  / nonascii

predicateIdentChar
  = [_a-z0-9-]i
  / nonascii

nonascii
  = [\x80-\uFFFF]

E "tab or space"
  = [ \t]

S "whitespace"
  = [ \t\r\n\f]
