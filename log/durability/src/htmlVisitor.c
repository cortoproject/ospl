/* $CORTO_GENERATED
 *
 * htmlVisitor.c
 *
 * Only code written between the begin and end tags will be preserved
 * when the file is regenerated.
 */

#include <include/durability.h>

corto_void _durability_htmlVisitor__matched(
    durability_htmlVisitor this,
    durability_parser parser,
    corto_object node)
{
/* $begin(ospl/log/durability/htmlVisitor/_matched) */
    corto_value t, o = corto_value_object(node, NULL);

    if (!corto_value_getMember(&o, "header.t", &t)) {
        this->lastReportedTime = *(corto_time*)corto_value_getPtr(&t);
    } else if (!corto_value_getMember(&o, "t", &t)) {
        corto_lasterr(); /* Catch error from previous getMember */
        this->lastReportedTime = *(corto_time*)corto_value_getPtr(&t);
    }

/* $end */
}

corto_void _durability_htmlVisitor_destruct(
    durability_htmlVisitor this)
{
/* $begin(ospl/log/durability/htmlVisitor/destruct) */
    FILE *f = fopen("index.html", "w");
    if (!f) {
        corto_error("cannot open index.html: %s", corto_lasterr);
    } else {
        /* Set timestamps of fellows that are still alive */
        durability_fellowTracker_stop(&this->tracker, &this->lastReportedTime);

        char *data = corto_contentof(NULL, "text/json", this);
        fprintf(f, "<html>\n");
        fprintf(f, "  <head><script src=\"https://d3js.org/d3.v4.min.js\"></script></head>\n");
        fprintf(f, "  <style>\n");
        fprintf(f, "    body {font-family: arial; font-size: 12; background-color: #eee; position: relative;}\n");
        fprintf(f, "    .labels {\n");
        fprintf(f, "      position: absolute;\n");
        fprintf(f, "      top: 0;\n");
        fprintf(f, "      left: 0;\n");
        fprintf(f, "    }\n");
        fprintf(f, "    .chart {   \n");
        fprintf(f, "      position: absolute;\n");
        fprintf(f, "      top: 0;\n");
        fprintf(f, "      left: 0;\n");
        fprintf(f, "    }\n");
        fprintf(f, "    .line { fill: #ccc; }\n");
        fprintf(f, "    .chart rect { fill: black; }\n");
        fprintf(f, "  </style>\n");
        fprintf(f, "  <body>\n");
        fprintf(f, "    <svg class=\"labels\"></svg>\n");
        fprintf(f, "    <svg id=\"chart\"></svg>\n");
        fprintf(f, "    <script>\n");
        fprintf(f, "      var data = %s\n", data);
        fprintf(f, "      var start = data.tracker.fellows[0].value.alive[0].from.sec;\n");
        fprintf(f, "      var end = 1485222762;    \n");
        fprintf(f, "      var barHeight = 20, \n");
        fprintf(f, "          barStroke = 3, \n");
        fprintf(f, "          barVMargin = 5,\n");
        fprintf(f, "          barHMargin = 2,\n");
        fprintf(f, "          barOffset = 80, \n");
        fprintf(f, "          padding = 10, \n");
        fprintf(f, "          paddingTop = 50, \n");
        fprintf(f, "          paddingBottom = 30;\n");
        fprintf(f, "      var chartWidth = document.documentElement.clientWidth - (padding + barOffset);\n");
        fprintf(f, "      var x = d3.scaleLinear()\n");
        fprintf(f, "        .domain([start, end])\n");
        fprintf(f, "        .range([0, chartWidth]);\n");
        fprintf(f, "      var xAxis = d3.axisTop(x).tickFormat(d3.format(\".0f\"));\n");
        fprintf(f, "      var labels = d3.select(\".labels\")\n");
        fprintf(f, "          .attr(\"width\", padding + barOffset)\n");
        fprintf(f, "          .attr(\"height\", (barHeight + barVMargin) * data.tracker.fellows.length + paddingTop + paddingBottom)\n");
        fprintf(f, "          .append(\"g\")\n");
        fprintf(f, "            .attr(\"transform\", \"translate(0, \" + paddingTop + \")\");\n");
        fprintf(f, "      var chart = d3.select(\"#chart\")\n");
        fprintf(f, "          .attr(\"width\", \"100%%\")\n");
        fprintf(f, "          .attr(\"height\", (barHeight + barVMargin) * data.tracker.fellows.length + paddingTop + paddingBottom)\n");
        fprintf(f, "          .append(\"g\")\n");
        fprintf(f, "            .attr(\"transform\", \"translate(0, \" + paddingTop + \")\");\n");
        fprintf(f, "      var fellow = chart.selectAll(\"g\")\n");
        fprintf(f, "          .data(data.tracker.fellows)\n");
        fprintf(f, "        .enter().append(\"g\")\n");
        fprintf(f, "          .attr(\"transform\", function(d, i) { return \"translate(\" + (padding + barOffset) + \",\" + i * (barHeight + barVMargin)  + \")\"; });\n");
        fprintf(f, "      var fellowSelect = fellow.selectAll(\"rect\")\n");
        fprintf(f, "          .data(function(d, i) { return data.tracker.fellows[i].value.alive; })\n");
        fprintf(f, "        .enter()\n");
        fprintf(f, "      fellowSelect.append(\"rect\")\n");
        fprintf(f, "            .attr(\"x\", 0)\n");
        fprintf(f, "            .attr(\"y\", barHeight / 2 - 1 / 2)\n");
        fprintf(f, "            .attr(\"width\", function(d, i) { return x(start + end - start) - barHMargin; })\n");
        fprintf(f, "            .attr(\"height\", 1)\n");
        fprintf(f, "            .attr(\"class\", \"line\");\n");
        fprintf(f, "      fellowSelect.append(\"rect\")\n");
        fprintf(f, "            .attr(\"x\", function(d, i) { console.log(\"x = \" + x(d.from.sec) + \", width = \" + x(d.to.sec - d.from.sec)); return x(d.from.sec) + barHMargin / 2; })\n");
        fprintf(f, "            .attr(\"y\", barHeight / 2 - barStroke / 2)\n");
        fprintf(f, "            .attr(\"width\", function(d, i) { return x(start + d.to.sec - d.from.sec) - barHMargin; })\n");
        fprintf(f, "            .attr(\"height\", barStroke);\n");
        fprintf(f, "      labels.selectAll(\"text\")\n");
        fprintf(f, "          .data(data.tracker.fellows)\n");
        fprintf(f, "        .enter().append(\"text\")\n");
        fprintf(f, "          .attr(\"x\", barOffset)\n");
        fprintf(f, "          .attr(\"y\", function(d, i) { return (i + 1) * (barHeight + barVMargin) - barVMargin - 6; })\n");
        fprintf(f, "          .text(function(d) { return d.value.id; })\n");
        fprintf(f, "          .attr(\"text-anchor\", \"end\");\n");
        fprintf(f, "      chart.append(\"g\")\n");
        fprintf(f, "            .attr(\"transform\", \"translate(\" + (padding + barOffset) + \", -20)\")\n");
        fprintf(f, "            .call(xAxis);\n");

        fprintf(f, "    </script>\n");
        fprintf(f, "  </body>\n");
        fprintf(f, "</html>\n");

        fclose(f);
    }

/* $end */
}

corto_void _durability_htmlVisitor_fellowLost(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_fellowLost *node)
{
/* $begin(ospl/log/durability/htmlVisitor/fellowLost) */

    if (durability_fellowTracker_lost(&this->tracker, node->fellow, &node->header.t)) {
        corto_error("%.9d.%.9d %s", 
            node->header.t.sec,
            node->header.t.nanosec,
            corto_lasterr());
    }

/* $end */
}

corto_void _durability_htmlVisitor_fellowNew(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_fellowNew *node)
{
/* $begin(ospl/log/durability/htmlVisitor/fellowNew) */

    durability_fellowTracker_new(&this->tracker, node->fellow, &node->header.t);

/* $end */
}

corto_void _durability_htmlVisitor_me(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_me *node)
{
/* $begin(ospl/log/durability/htmlVisitor/me) */

    this->myId = node->fellow;

/* $end */
}

corto_void _durability_htmlVisitor_terminating(
    durability_htmlVisitor this,
    durability_parser parser,
    durability_terminating *node)
{
/* $begin(ospl/log/durability/htmlVisitor/terminating) */

    durability_fellowTracker_stop(&this->tracker, &node->t);

/* $end */
}
