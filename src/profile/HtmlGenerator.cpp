/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2016, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

#include "profile/HtmlGenerator.h"
#include <sstream>
#include <string>

namespace souffle {
namespace profile {
namespace {
std::string cssChartist = R"___(
p {
    padding: 4px
}

th[role=columnheader]:not(.no-sort) {
    cursor: pointer
}

th[role=columnheader]:not(.no-sort):after {
    content: '';
    float: right;
    margin-top: 7px;
    border-width: 0 4px 4px;
    border-style: solid;
    border-color: #404040 transparent;
    visibility: hidden;
    opacity: 0;
    -webkit-user-select: none;
    -moz-user-select: none;
    user-select: none
}

.ct-double-octave:before,
.ct-major-eleventh:before,
.ct-major-second:before,
.ct-major-seventh:before,
.ct-major-sixth:before,
.ct-major-tenth:before,
.ct-major-third:before,
.ct-major-twelfth:before,
.ct-minor-second:before,
.ct-minor-seventh:before,
.ct-minor-sixth:before,
.ct-minor-third:before,
.ct-octave:before,
.ct-perfect-fifth:before,
.ct-perfect-fourth:before,
.ct-square:before {
    float: left;
    content: "";
    height: 0
}

th[aria-sort=ascending]:not(.no-sort):after {
    border-bottom: none;
    border-width: 4px 4px 0
}

th[aria-sort]:not(.no-sort):after {
    visibility: visible;
    opacity: .4
}

th[role=columnheader]:not(.no-sort):hover:after {
    visibility: visible;
    opacity: 1
}

.ct-double-octave:after,
.ct-major-eleventh:after,
.ct-major-second:after,
.ct-major-seventh:after,
.ct-major-sixth:after,
.ct-major-tenth:after,
.ct-major-third:after,
.ct-major-twelfth:after,
.ct-minor-second:after,
.ct-minor-seventh:after,
.ct-minor-sixth:after,
.ct-minor-third:after,
.ct-octave:after,
.ct-perfect-fifth:after,
.ct-perfect-fourth:after,
.ct-square:after {
    content: "";
    clear: both
}

.ct-label {
    fill: rgba(0, 0, 0, .4);
    color: rgba(0, 0, 0, .4);
    font-size: .75rem;
    line-height: 1
}

.ct-grid-background,
.ct-line {
    fill: none
}

.ct-chart-bar .ct-label,
.ct-chart-line .ct-label {
    display: block;
    display: -webkit-box;
    display: -moz-box;
    display: -ms-flexbox;
    display: -webkit-flex;
    display: flex
}

.ct-chart-donut .ct-label,
.ct-chart-pie .ct-label {
    dominant-baseline: central
}

.ct-label.ct-horizontal.ct-start {
    -webkit-box-align: flex-end;
    -webkit-align-items: flex-end;
    -ms-flex-align: flex-end;
    align-items: flex-end;
    -webkit-box-pack: flex-start;
    -webkit-justify-content: flex-start;
    -ms-flex-pack: flex-start;
    justify-content: flex-start;
    text-align: left;
    text-anchor: start
}

.ct-label.ct-horizontal.ct-end {
    -webkit-box-align: flex-start;
    -webkit-align-items: flex-start;
    -ms-flex-align: flex-start;
    align-items: flex-start;
    -webkit-box-pack: flex-start;
    -webkit-justify-content: flex-start;
    -ms-flex-pack: flex-start;
    justify-content: flex-start;
    text-align: left;
    text-anchor: start
}

.ct-label.ct-vertical.ct-start {
    -webkit-box-align: flex-end;
    -webkit-align-items: flex-end;
    -ms-flex-align: flex-end;
    align-items: flex-end;
    -webkit-box-pack: flex-end;
    -webkit-justify-content: flex-end;
    -ms-flex-pack: flex-end;
    justify-content: flex-end;
    text-align: right;
    text-anchor: end
}

.ct-label.ct-vertical.ct-end {
    -webkit-box-align: flex-end;
    -webkit-align-items: flex-end;
    -ms-flex-align: flex-end;
    align-items: flex-end;
    -webkit-box-pack: flex-start;
    -webkit-justify-content: flex-start;
    -ms-flex-pack: flex-start;
    justify-content: flex-start;
    text-align: left;
    text-anchor: start
}

.ct-chart-bar .ct-label.ct-horizontal.ct-start {
    -webkit-box-align: flex-end;
    -webkit-align-items: flex-end;
    -ms-flex-align: flex-end;
    align-items: flex-end;
    -webkit-box-pack: center;
    -webkit-justify-content: center;
    -ms-flex-pack: center;
    justify-content: center;
    text-align: center;
    text-anchor: start
}

.ct-chart-bar .ct-label.ct-horizontal.ct-end {
    -webkit-box-align: flex-start;
    -webkit-align-items: flex-start;
    -ms-flex-align: flex-start;
    align-items: flex-start;
    -webkit-box-pack: center;
    -webkit-justify-content: center;
    -ms-flex-pack: center;
    justify-content: center;
    text-align: center;
    text-anchor: start
}

.ct-chart-bar.ct-horizontal-bars .ct-label.ct-horizontal.ct-start {
    -webkit-box-align: flex-end;
    -webkit-align-items: flex-end;
    -ms-flex-align: flex-end;
    align-items: flex-end;
    -webkit-box-pack: flex-start;
    -webkit-justify-content: flex-start;
    -ms-flex-pack: flex-start;
    justify-content: flex-start;
    text-align: left;
    text-anchor: start
}

.ct-chart-bar.ct-horizontal-bars .ct-label.ct-horizontal.ct-end {
    -webkit-box-align: flex-start;
    -webkit-align-items: flex-start;
    -ms-flex-align: flex-start;
    align-items: flex-start;
    -webkit-box-pack: flex-start;
    -webkit-justify-content: flex-start;
    -ms-flex-pack: flex-start;
    justify-content: flex-start;
    text-align: left;
    text-anchor: start
}

.ct-chart-bar.ct-horizontal-bars .ct-label.ct-vertical.ct-start {
    -webkit-box-align: center;
    -webkit-align-items: center;
    -ms-flex-align: center;
    align-items: center;
    -webkit-box-pack: flex-end;
    -webkit-justify-content: flex-end;
    -ms-flex-pack: flex-end;
    justify-content: flex-end;
    text-align: right;
    text-anchor: end
}

.ct-chart-bar.ct-horizontal-bars .ct-label.ct-vertical.ct-end {
    -webkit-box-align: center;
    -webkit-align-items: center;
    -ms-flex-align: center;
    align-items: center;
    -webkit-box-pack: flex-start;
    -webkit-justify-content: flex-start;
    -ms-flex-pack: flex-start;
    justify-content: flex-start;
    text-align: left;
    text-anchor: end
}

.ct-grid {
    stroke: rgba(0, 0, 0, .2);
    stroke-width: 1px;
    stroke-dasharray: 2px
}

.ct-point {
    stroke-width: 10px;
    stroke-linecap: round
}

.ct-line {
    stroke-width: 4px
}

.ct-area {
    stroke: none;
    fill-opacity: .1
}

.ct-bar {
    fill: none;
    stroke-width: 10px
}

.ct-slice-donut {
    fill: none;
    stroke-width: 60px
}

.ct-series-a .ct-bar,
.ct-series-a .ct-line,
.ct-series-a .ct-point,
.ct-series-a .ct-slice-donut {
    stroke: #d70206
}

.ct-series-a .ct-area,
.ct-series-a .ct-slice-pie {
    fill: #d70206
}

.ct-series-b .ct-bar,
.ct-series-b .ct-line,
.ct-series-b .ct-point,
.ct-series-b .ct-slice-donut {
    stroke: #f05b4f
}

.ct-series-b .ct-area,
.ct-series-b .ct-slice-pie {
    fill: #f05b4f
}

.ct-series-c .ct-bar,
.ct-series-c .ct-line,
.ct-series-c .ct-point,
.ct-series-c .ct-slice-donut {
    stroke: #f4c63d
}

.ct-series-c .ct-area,
.ct-series-c .ct-slice-pie {
    fill: #f4c63d
}

.ct-series-d .ct-bar,
.ct-series-d .ct-line,
.ct-series-d .ct-point,
.ct-series-d .ct-slice-donut {
    stroke: #d17905
}

.ct-series-d .ct-area,
.ct-series-d .ct-slice-pie {
    fill: #d17905
}

.ct-series-e .ct-bar,
.ct-series-e .ct-line,
.ct-series-e .ct-point,
.ct-series-e .ct-slice-donut {
    stroke: #453d3f
}

.ct-series-e .ct-area,
.ct-series-e .ct-slice-pie {
    fill: #453d3f
}

.ct-series-f .ct-bar,
.ct-series-f .ct-line,
.ct-series-f .ct-point,
.ct-series-f .ct-slice-donut {
    stroke: #59922b
}

.ct-series-f .ct-area,
.ct-series-f .ct-slice-pie {
    fill: #59922b
}

.ct-series-g .ct-bar,
.ct-series-g .ct-line,
.ct-series-g .ct-point,
.ct-series-g .ct-slice-donut {
    stroke: #0544d3
}

.ct-series-g .ct-area,
.ct-series-g .ct-slice-pie {
    fill: #0544d3
}

.ct-series-h .ct-bar,
.ct-series-h .ct-line,
.ct-series-h .ct-point,
.ct-series-h .ct-slice-donut {
    stroke: #6b0392
}

.ct-series-h .ct-area,
.ct-series-h .ct-slice-pie {
    fill: #6b0392
}

.ct-series-i .ct-bar,
.ct-series-i .ct-line,
.ct-series-i .ct-point,
.ct-series-i .ct-slice-donut {
    stroke: #f05b4f
}

.ct-series-i .ct-area,
.ct-series-i .ct-slice-pie {
    fill: #f05b4f
}

.ct-series-j .ct-bar,
.ct-series-j .ct-line,
.ct-series-j .ct-point,
.ct-series-j .ct-slice-donut {
    stroke: #dda458
}

.ct-series-j .ct-area,
.ct-series-j .ct-slice-pie {
    fill: #dda458
}

.ct-series-k .ct-bar,
.ct-series-k .ct-line,
.ct-series-k .ct-point,
.ct-series-k .ct-slice-donut {
    stroke: #eacf7d
}

.ct-series-k .ct-area,
.ct-series-k .ct-slice-pie {
    fill: #eacf7d
}

.ct-series-l .ct-bar,
.ct-series-l .ct-line,
.ct-series-l .ct-point,
.ct-series-l .ct-slice-donut {
    stroke: #86797d
}

.ct-series-l .ct-area,
.ct-series-l .ct-slice-pie {
    fill: #86797d
}

.ct-series-m .ct-bar,
.ct-series-m .ct-line,
.ct-series-m .ct-point,
.ct-series-m .ct-slice-donut {
    stroke: #b2c326
}

.ct-series-m .ct-area,
.ct-series-m .ct-slice-pie {
    fill: #b2c326
}

.ct-series-n .ct-bar,
.ct-series-n .ct-line,
.ct-series-n .ct-point,
.ct-series-n .ct-slice-donut {
    stroke: #6188e2
}

.ct-series-n .ct-area,
.ct-series-n .ct-slice-pie {
    fill: #6188e2
}

.ct-series-o .ct-bar,
.ct-series-o .ct-line,
.ct-series-o .ct-point,
.ct-series-o .ct-slice-donut {
    stroke: #a748ca
}

.ct-series-o .ct-area,
.ct-series-o .ct-slice-pie {
    fill: #a748ca
}

.ct-square {
    display: block;
    position: relative;
    width: 100%
}

.ct-square:before {
    display: block;
    width: 0;
    padding-bottom: 100%
}

.ct-square:after {
    display: table
}

.ct-square>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-minor-second {
    display: block;
    position: relative;
    width: 100%
}

.ct-minor-second:before {
    display: block;
    width: 0;
    padding-bottom: 93.75%
}

.ct-minor-second:after {
    display: table
}

.ct-minor-second>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-major-second {
    display: block;
    position: relative;
    width: 100%
}

.ct-major-second:before {
    display: block;
    width: 0;
    padding-bottom: 88.8888888889%
}

.ct-major-second:after {
    display: table
}

.ct-major-second>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-minor-third {
    display: block;
    position: relative;
    width: 100%
}

.ct-minor-third:before {
    display: block;
    width: 0;
    padding-bottom: 83.3333333333%
}

.ct-minor-third:after {
    display: table
}

.ct-minor-third>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-major-third {
    display: block;
    position: relative;
    width: 100%
}

.ct-major-third:before {
    display: block;
    width: 0;
    padding-bottom: 80%
}

.ct-major-third:after {
    display: table
}

.ct-major-third>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-perfect-fourth {
    display: block;
    position: relative;
    width: 100%
}

.ct-perfect-fourth:before {
    display: block;
    width: 0;
    padding-bottom: 75%
}

.ct-perfect-fourth:after {
    display: table
}

.ct-perfect-fourth>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-perfect-fifth {
    display: block;
    position: relative;
    width: 100%
}

.ct-perfect-fifth:before {
    display: block;
    width: 0;
    padding-bottom: 66.6666666667%
}

.ct-perfect-fifth:after {
    display: table
}

.ct-perfect-fifth>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-minor-sixth {
    display: block;
    position: relative;
    width: 100%
}

.ct-minor-sixth:before {
    display: block;
    width: 0;
    padding-bottom: 62.5%
}

.ct-minor-sixth:after {
    display: table
}

.ct-minor-sixth>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-golden-section {
    display: block;
    position: relative;
    width: 100%
}

.ct-golden-section:before {
    display: block;
    float: left;
    content: "";
    width: 0;
    height: 0;
    padding-bottom: 61.804697157%
}

.ct-golden-section:after {
    content: "";
    display: table;
    clear: both
}

.ct-golden-section>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-major-sixth {
    display: block;
    position: relative;
    width: 100%
}

.ct-major-sixth:before {
    display: block;
    width: 0;
    padding-bottom: 60%
}

.ct-major-sixth:after {
    display: table
}

.ct-major-sixth>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-minor-seventh {
    display: block;
    position: relative;
    width: 100%
}

.ct-minor-seventh:before {
    display: block;
    width: 0;
    padding-bottom: 56.25%
}

.ct-minor-seventh:after {
    display: table
}

.ct-minor-seventh>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-major-seventh {
    display: block;
    position: relative;
    width: 100%
}

.ct-major-seventh:before {
    display: block;
    width: 0;
    padding-bottom: 53.3333333333%
}

.ct-major-seventh:after {
    display: table
}

.ct-major-seventh>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-octave {
    display: block;
    position: relative;
    width: 100%
}

.ct-octave:before {
    display: block;
    width: 0;
    padding-bottom: 50%
}

.ct-octave:after {
    display: table
}

.ct-octave>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-major-tenth {
    display: block;
    position: relative;
    width: 100%
}

.ct-major-tenth:before {
    display: block;
    width: 0;
    padding-bottom: 40%
}

.ct-major-tenth:after {
    display: table
}

.ct-major-tenth>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-major-eleventh {
    display: block;
    position: relative;
    width: 100%
}

.ct-major-eleventh:before {
    display: block;
    width: 0;
    padding-bottom: 37.5%
}

.ct-major-eleventh:after {
    display: table
}

.ct-major-eleventh>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-major-twelfth {
    display: block;
    position: relative;
    width: 100%
}

.ct-major-twelfth:before {
    display: block;
    width: 0;
    padding-bottom: 33.3333333333%
}

.ct-major-twelfth:after {
    display: table
}

.ct-major-twelfth>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}

.ct-double-octave {
    display: block;
    position: relative;
    width: 100%
}

.ct-double-octave:before {
    display: block;
    width: 0;
    padding-bottom: 25%
}

.ct-double-octave:after {
    display: table
}

.ct-double-octave>svg {
    display: block;
    position: absolute;
    top: 0;
    left: 0
}
)___";
std::string cssStyle = R"___(
a, abbr, acronym, address,
applet, article, aside, audio, b, big, blockquote, body, canvas, caption,
center, cite, code, dd, del, details, dfn, dl, dt, em, embed, fieldset,
figcaption, figure, footer, form, h1, h2, h3, h4, h5, h6, header, html,
i, iframe, img, ins, kbd, label, legend, li, mark, menu, nav,
object, ol, output, p, pre, q, ruby, s, samp, section, small,
span, strike, strong, sub, summary, sup, table, tbody, td,
tfoot, th, thead, time, tr, tt, u, ul, var, video {
    margin: 0;
    padding: 0;
    border: 0;
    font: inherit;
    vertical-align: baseline;
}

th[role=columnheader]:not(.no-sort) {
    cursor: pointer;
}

th[role=columnheader]:not(.no-sort):after {
    content: '';
    float: right;
    margin-top: 7px;
    border-width: 0 4px 4px;
    border-style: solid;
    border-color: #404040 transparent;
    visibility: hidden;
    opacity: 0;
    -webkit-user-select: none;
    -moz-user-select: none;
    user-select: none;
}

th[aria-sort=ascending]:not(.no-sort):after {
    border-bottom: none;
    border-width: 4px 4px 0;
}

th[aria-sort]:not(.no-sort):after {
    visibility: visible;
    opacity: 0.4;
}

th[role=columnheader]:not(.no-sort):hover:after {
    visibility: visible;
    opacity: 1;
}

body,
h1 {
    font-size: 13px
}

article, aside, details, figcaption, figure, footer, header, menu, nav, section {
    display: block
}

body,
html {
    line-height: 1
}

ol,
ul {
    list-style: none
}

blockquote,
q {
    quotes: none
}

blockquote:after,
blockquote:before,
q:after,
q:before {
    content: none
}

:focus {
    outline: 0
}

*,
:after,
:before {
    box-sizing: border-box
}

body {
    margin: 0;
    font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif;
    line-height: 18px;
    color: #303030;
    background-color: #fafafa;
    -webkit-font-smoothing: antialiased;
    padding: 0
}

h1,
h2,
h3,
h4,
h5 {
    font-weight: 700;
    display: block;
    margin: 0 0 10px;
    text-align: center;
}

h1,
ul {
    margin: 0 0 20px
}

h1 {
    display: block;
    font-weight: 400;
    text-shadow: 0 1px 0 #fff
}

a,
strong,
table th {
    font-weight: 700
}

h1 span.description {
    color: #6d6d6d
}

h2 {
    font-size: 18px;
    line-height: 24px;
    margin: 20px 0 10px
}

h3 {
    font-size: 15px;
    margin: 20px 0
}

li {
    margin-left: 30px;
    margin-bottom: 3px
}

ul li {
    list-style: disc
}

ol li {
    list-style: decimal
}

a {
    color: #404040;
    text-decoration: none;
    border-bottom: 1px solid #ddd
}

a:hover {
    border-color: #d0d0d0
}

.notice {
    background: #ffa;
    border: 1px solid #cc7;
    display: block;
    padding: 10px;
    margin-bottom: 10px
}

.stretch {
    display: block;
    width: 100%
}

.pad1y {
    padding: 10px 0
}

.center {
    text-align: center
}

.content {
    margin-top: 40px;
    padding: 0 0 20px
}

table {
    background: #fff;
    max-width: 100%;
    border-spacing: 0;
    width: 100%;
    margin: 10px 0;
    border: 1px solid #ddd;
    border-collapse: separate;
    -webkit-box-shadow: 0 0 4px rgba(0, 0, 0, .1);
    -moz-box-shadow: 0 0 4px rgba(0, 0, 0, .1);
    box-shadow: 0 0 4px rgba(0, 0, 0, .1)
}

table td,
table th {
    position: relative;
    padding: 4px;
    line-height: 15px;
    text-align: left;
    border-top: 1px solid #ddd
}

.limiter,
header {
    margin: 0 auto;
    padding: 0 20px
}

table th {
    background: #eee;
    background: -webkit-gradient(linear, left top, left bottom, from(#f6f6f6), to(#eee));
    background: -moz-linear-gradient(top, #f6f6f6, #eee);
    text-shadow: 0 1px 0 #fff;
    vertical-align: bottom
}

table td {
    vertical-align: top
}

table tr {
    background: rgba(0, 255, 0, 0)
}

table tbody:first-child tr:first-child td,
table tbody:first-child tr:first-child th,
table thead:first-child tr td,
table thead:first-child tr th,
table thead:first-child tr:first-child th {
    border-top: 0
}

table tbody + tbody {
    border-top: 2px solid #ddd
}

table td + td,
table td + th,
table th + td,
table th + th {
    border-left: 1px solid #ddd
}

header {
    width: 960px
}

.limiter {
    width: 520px
}

.links {
    width: 480px;
    margin: 50px auto 0
}

.links a {
    width: 50%;
    float: left
}

a.button {
    background: #1F90FF;
    border: 1px solid #1f4fff;
    height: 40px;
    line-height: 38px;
    color: #fff;
    display: inline-block;
    text-align: center;
    padding: 0 10px;
    -webkit-border-radius: 1px;
    border-radius: 1px;
    -webkit-transition: box-shadow 150ms linear;
    -moz-transition: box-shadow 150ms linear;
    -o-transition: box-shadow 150ms linear;
    transition: box-shadow 150ms linear
}

pre,
pre code {
    line-height: 1.25em
}

a.button:hover {
background-color: #0081ff;
-webkit-box-shadow: 0 1px 5px rgba(0, 0, 0, .25);
box-shadow: 0 1px 5px rgba(0, 0, 0, .25);
border: 1px solid #1f4fff
}

a.button:active,
a.button:focus {
background: #0081ff;
-webkit-box-shadow: inset 0 1px 5px rgba(0, 0, 0, .25);
box-shadow: inset 0 1px 5px rgba(0, 0, 0, .25)
}

.options {
    margin: 10px 0 30px 15px
}

.options h3 {
    display: block;
    padding-top: 10px;
    margin-top: 20px
}

.options h3:first-child {
    border: none;
    margin-top: 0
}

code,
pre {
    font-family: Consolas, Menlo, 'Liberation Mono', Courier, monospace;
    word-wrap: break-word;
    color: #333
}

pre {
    font-size: 13px;
    background: #fff;
    padding: 10px 15px;
    margin: 10px 0;
    overflow: auto;
    -webkit-box-shadow: 0 1px 3px rgba(0, 0, 0, .3);
    box-shadow: 0 1px 3px rgba(0, 0, 0, .3)
}

code {
    font-size: 12px;
    border: 0;
    padding: 0;
    background: #e6e6e6;
    background: rgba(0, 0, 0, .08);
    box-shadow: 0 0 0 2px rgba(0, 0, 0, .08)
}

pre code {
    font-size: 13px;
    background: 0 0;
    box-shadow: none;
    border: none;
    padding: 0;
    margin: 0
}

.col12 {
    width: 100%
}

.col6 {
    width: 50%;
    float: left;
    display: block
}

#toolbar-button,
ul.tab li a {
    display: inline-block;
    text-align: center;
    text-decoration: none
}

.pill-group {
    margin: 40px 0 0
}

.pill-group a:first-child {
    border-radius: 20px 0 0 20px;
    border-right-width: 0
}

.pill-group a:last-child {
    border-radius: 0 20px 20px 0
}

#toolbar-button {
    background-color: #4CAF50;
    border: none;
    color: #fff;
    margin: 7px 12px;
    padding: 8px 14px;
    font-size: 32px
}

.tabcontent,
ul.tab {
    border: 1px solid #ccc
}

#toolbar-button:hover {
    background-color: #0081ff;
    -webkit-box-shadow: 0 1px 5px rgba(0, 0, 0, .25);
    box-shadow: 0 1px 5px rgba(0, 0, 0, .25)
}

#toolbar-button:active,
#toolbar-button:focus {
    background: #0081ff;
    -webkit-box-shadow: inset 0 1px 5px rgba(0, 0, 0, .25);
    box-shadow: inset 0 1px 5px rgba(0, 0, 0, .25)
}

ul.tab {
    margin: 0;
    padding: 0;
    overflow: hidden;
    border: 1px solid #ccc;
    background-color: #f1f1f1;
}

ul.tab li {
    float: left;
    list-style: none;
}

ul.tab li a {
    display: inline-block;
    color: black;
    text-align: center;
    padding: 14px 16px;
    text-decoration: none;
    transition: 0.3s;
    font-size: 17px;
}

ul.tab li a:hover {
    background-color: #ddd;
}

ul.tab li a:focus, .active {
    background-color: #ccc;
}

.tabcontent {
    display: none;
    padding: 6px 12px;
    border: 1px solid #ccc;
    border-top: none;
}

.rulesofrel {
    display: none;
    animation: fadeEffect 1s
}

.RulVerTable {
    display: none;
    -webkit-animation: fadeEffect 1s;
    animation: fadeEffect 1s
}

@-webkit-keyframes fadeEffect {
    from {
        opacity: 0
    }
    to {
        opacity: 1
    }
}

@keyframes fadeEffect {
    from {
        opacity: 0
    }
    to {
        opacity: 1
    }
}

.rulesofrel,
.tabcontent {
    -webkit-animation: fadeEffect 1s
}

.perc_time,
.perc_tup {
    padding: 0;
    margin: 0;
    background: #aaf;
    width: 100%;
    height: 30px;
    font: inherit;
    font-weight: 700;
    color: #000
}

.text_cell {
    position: relative;
}

.text_cell span {
    word-break: break-all;
    position: absolute;
    width: 95%;
    text-overflow: ellipsis;
    white-space: nowrap;
    overflow: hidden;
}

.text_cell span:hover {
    word-break: break-all;
    position: absolute;
    width: 95%;
    height: auto;
    text-overflow: initial;
    white-space: normal;
    overflow: visible;
    background: #fff;
    z-index: 10;
}

th:last-child {
    width: 20%;
}

th:first-child {
    width: 80%;
}

.table_wrapper {
    max-height: 50vh;
    overflow-y: scroll;
}

/* chartist tooltip plugin css */
.chartist-tooltip {
    position: absolute;
    display: none;
    opacity: 0;
    min-width: 5em;
    padding: .5em;
    background: #F4C63D;
    color: #453D3F;
    font-family: Oxygen, Helvetica, Arial, sans-serif;
    font-weight: 700;
    text-align: center;
    pointer-events: none;
    z-index: 1;
    -webkit-transition: opacity .2s linear;
    -moz-transition: opacity .2s linear;
    -o-transition: opacity .2s linear;
    transition: opacity .2s linear;
}

.chartist-tooltip:before {

    content: "";
    position: absolute;
    top: 100%;
    left: 50%;
    width: 0;
    height: 0;
    margin-left: -15px;
    border: 15px solid transparent;
    border-top-color: #F4C63D;
}

.chartist-tooltip.tooltip-show {
    display: inline-block;
    opacity: 1;
}

.ct-area, .ct-line {
    pointer-events: none;
}

button {
    font-family: inherit;
    font-size: 100%;
    padding: .5em 1em;
    color: #444;
    color: rgba(0, 0, 0, .8);
    border: 1px solid #999;
    background-color: #E6E6E6;
    text-decoration: none;
    border-radius: 2px
}
button:hover {
    filter: alpha(opacity=90);
    background-image: -webkit-linear-gradient(transparent, rgba(0, 0, 0, .05) 40%, rgba(0, 0, 0, .1));
    background-image: linear-gradient(transparent, rgba(0, 0, 0, .05) 40%, rgba(0, 0, 0, .1))
}

button {
    border: none;
    display: inline-block;
    zoom: 1;
    line-height: normal;
    white-space: nowrap;
    vertical-align: middle;
    text-align: center;
    cursor: pointer;
    -webkit-user-drag: none;
    -webkit-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    box-sizing: border-box;
}

#Help p {
    margin-bottom: 1em;
}

#code-list {
    background: #AAA;
    padding-left: 2em;
    color: #666;
}

.code-li {
    background: #FAFAFA;
    marginBottom: 0;
}

#code-view {
    overflow: auto;
    height: calc( 100vh - 160px );
    width: calc( 100vw - 25px );
    font-family: Consolas, Menlo, Monaco, Lucida Console,'Bitstream Vera Sans Mono','Courier',monospace;
    line-height: 21px;
}

#code-view .text-span {
    white-space: nowrap;
    padding-left: 6px;
    color: #666;
}

#code-view .ol li:before  {
    color: #666;
    background: #AAA;
}

.code-li:hover {
    background: #eacf7d;
}

.number-span {
    content: counter(item) ". ";
    counter-increment: item;
    list-style:decimal;
    width: 60px;
}
)___";
std::string jsChartistMin = R"___(
!function(a,b){"function"==typeof define&&define.amd?define("Chartist",[],function(){return a.Chartist=b()}):"object"==typeof exports?module.exports=b():a.Chartist=b()}(this,function(){var a={version:"0.10.0"};return function(a,b,c){"use strict";c.namespaces={svg:"http://www.w3.org/2000/svg",xmlns:"http://www.w3.org/2000/xmlns/",xhtml:"http://www.w3.org/1999/xhtml",xlink:"http://www.w3.org/1999/xlink",ct:"http://gionkunz.github.com/chartist-js/ct"},c.noop=function(a){return a},c.alphaNumerate=function(a){return String.fromCharCode(97+a%26)},c.extend=function(a){var b,d,e;for(a=a||{},b=1;b<arguments.length;b++){d=arguments[b];for(var f in d)e=d[f],"object"!=typeof e||null===e||e instanceof Array?a[f]=e:a[f]=c.extend(a[f],e)}return a},c.replaceAll=function(a,b,c){return a.replace(new RegExp(b,"g"),c)},c.ensureUnit=function(a,b){return"number"==typeof a&&(a+=b),a},c.quantity=function(a){if("string"==typeof a){var b=/^(\d+)\s*(.*)$/g.exec(a);return{value:+b[1],unit:b[2]||void 0}}return{value:a}},c.querySelector=function(a){return a instanceof Node?a:b.querySelector(a)},c.times=function(a){return Array.apply(null,new Array(a))},c.sum=function(a,b){return a+(b?b:0)},c.mapMultiply=function(a){return function(b){return b*a}},c.mapAdd=function(a){return function(b){return b+a}},c.serialMap=function(a,b){var d=[],e=Math.max.apply(null,a.map(function(a){return a.length}));return c.times(e).forEach(function(c,e){var f=a.map(function(a){return a[e]});d[e]=b.apply(null,f)}),d},c.roundWithPrecision=function(a,b){var d=Math.pow(10,b||c.precision);return Math.round(a*d)/d},c.precision=8,c.escapingMap={"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#039;"},c.serialize=function(a){return null===a||void 0===a?a:("number"==typeof a?a=""+a:"object"==typeof a&&(a=JSON.stringify({data:a})),Object.keys(c.escapingMap).reduce(function(a,b){return c.replaceAll(a,b,c.escapingMap[b])},a))},c.deserialize=function(a){if("string"!=typeof a)return a;a=Object.keys(c.escapingMap).reduce(function(a,b){return c.replaceAll(a,c.escapingMap[b],b)},a);try{a=JSON.parse(a),a=void 0!==a.data?a.data:a}catch(b){}return a},c.createSvg=function(a,b,d,e){var f;return b=b||"100%",d=d||"100%",Array.prototype.slice.call(a.querySelectorAll("svg")).filter(function(a){return a.getAttributeNS(c.namespaces.xmlns,"ct")}).forEach(function(b){a.removeChild(b)}),f=new c.Svg("svg").attr({width:b,height:d}).addClass(e).attr({style:"width: "+b+"; height: "+d+";"}),a.appendChild(f._node),f},c.normalizeData=function(a,b,d){var e,f={raw:a,normalized:{}};return f.normalized.series=c.getDataArray({series:a.series||[]},b,d),e=f.normalized.series.every(function(a){return a instanceof Array})?Math.max.apply(null,f.normalized.series.map(function(a){return a.length})):f.normalized.series.length,f.normalized.labels=(a.labels||[]).slice(),Array.prototype.push.apply(f.normalized.labels,c.times(Math.max(0,e-f.normalized.labels.length)).map(function(){return""})),b&&c.reverseData(f.normalized),f},c.safeHasProperty=function(a,b){return null!==a&&"object"==typeof a&&a.hasOwnProperty(b)},c.isDataHoleValue=function(a){return null===a||void 0===a||"number"==typeof a&&isNaN(a)},c.reverseData=function(a){a.labels.reverse(),a.series.reverse();for(var b=0;b<a.series.length;b++)"object"==typeof a.series[b]&&void 0!==a.series[b].data?a.series[b].data.reverse():a.series[b]instanceof Array&&a.series[b].reverse()},c.getDataArray=function(a,b,d){function e(a){if(c.safeHasProperty(a,"value"))return e(a.value);if(c.safeHasProperty(a,"data"))return e(a.data);if(a instanceof Array)return a.map(e);if(!c.isDataHoleValue(a)){if(d){var b={};return"string"==typeof d?b[d]=c.getNumberOrUndefined(a):b.y=c.getNumberOrUndefined(a),b.x=a.hasOwnProperty("x")?c.getNumberOrUndefined(a.x):b.x,b.y=a.hasOwnProperty("y")?c.getNumberOrUndefined(a.y):b.y,b}return c.getNumberOrUndefined(a)}}return a.series.map(e)},c.normalizePadding=function(a,b){return b=b||0,"number"==typeof a?{top:a,right:a,bottom:a,left:a}:{top:"number"==typeof a.top?a.top:b,right:"number"==typeof a.right?a.right:b,bottom:"number"==typeof a.bottom?a.bottom:b,left:"number"==typeof a.left?a.left:b}},c.getMetaData=function(a,b){var c=a.data?a.data[b]:a[b];return c?c.meta:void 0},c.orderOfMagnitude=function(a){return Math.floor(Math.log(Math.abs(a))/Math.LN10)},c.projectLength=function(a,b,c){return b/c.range*a},c.getAvailableHeight=function(a,b){return Math.max((c.quantity(b.height).value||a.height())-(b.chartPadding.top+b.chartPadding.bottom)-b.axisX.offset,0)},c.getHighLow=function(a,b,d){function e(a){if(void 0!==a)if(a instanceof Array)for(var b=0;b<a.length;b++)e(a[b]);else{var c=d?+a[d]:+a;g&&c>f.high&&(f.high=c),h&&c<f.low&&(f.low=c)}}b=c.extend({},b,d?b["axis"+d.toUpperCase()]:{});var f={high:void 0===b.high?-Number.MAX_VALUE:+b.high,low:void 0===b.low?Number.MAX_VALUE:+b.low},g=void 0===b.high,h=void 0===b.low;return(g||h)&&e(a),(b.referenceValue||0===b.referenceValue)&&(f.high=Math.max(b.referenceValue,f.high),f.low=Math.min(b.referenceValue,f.low)),f.high<=f.low&&(0===f.low?f.high=1:f.low<0?f.high=0:f.high>0?f.low=0:(f.high=1,f.low=0)),f},c.isNumeric=function(a){return null!==a&&isFinite(a)},c.isFalseyButZero=function(a){return!a&&0!==a},c.getNumberOrUndefined=function(a){return c.isNumeric(a)?+a:void 0},c.isMultiValue=function(a){return"object"==typeof a&&("x"in a||"y"in a)},c.getMultiValue=function(a,b){return c.isMultiValue(a)?c.getNumberOrUndefined(a[b||"y"]):c.getNumberOrUndefined(a)},c.rho=function(a){function b(a,c){return a%c===0?c:b(c,a%c)}function c(a){return a*a+1}if(1===a)return a;var d,e=2,f=2;if(a%2===0)return 2;do e=c(e)%a,f=c(c(f))%a,d=b(Math.abs(e-f),a);while(1===d);return d},c.getBounds=function(a,b,d,e){function f(a,b){return a===(a+=b)&&(a*=1+(b>0?o:-o)),a}var g,h,i,j=0,k={high:b.high,low:b.low};k.valueRange=k.high-k.low,k.oom=c.orderOfMagnitude(k.valueRange),k.step=Math.pow(10,k.oom),k.min=Math.floor(k.low/k.step)*k.step,k.max=Math.ceil(k.high/k.step)*k.step,k.range=k.max-k.min,k.numberOfSteps=Math.round(k.range/k.step);var l=c.projectLength(a,k.step,k),m=l<d,n=e?c.rho(k.range):0;if(e&&c.projectLength(a,1,k)>=d)k.step=1;else if(e&&n<k.step&&c.projectLength(a,n,k)>=d)k.step=n;else for(;;){if(m&&c.projectLength(a,k.step,k)<=d)k.step*=2;else{if(m||!(c.projectLength(a,k.step/2,k)>=d))break;if(k.step/=2,e&&k.step%1!==0){k.step*=2;break}}if(j++>1e3)throw new Error("Exceeded maximum number of iterations while optimizing scale step!")}var o=2.221e-16;for(k.step=Math.max(k.step,o),h=k.min,i=k.max;h+k.step<=k.low;)h=f(h,k.step);for(;i-k.step>=k.high;)i=f(i,-k.step);k.min=h,k.max=i,k.range=k.max-k.min;var p=[];for(g=k.min;g<=k.max;g=f(g,k.step)){var q=c.roundWithPrecision(g);q!==p[p.length-1]&&p.push(q)}return k.values=p,k},c.polarToCartesian=function(a,b,c,d){var e=(d-90)*Math.PI/180;return{x:a+c*Math.cos(e),y:b+c*Math.sin(e)}},c.createChartRect=function(a,b,d){var e=!(!b.axisX&&!b.axisY),f=e?b.axisY.offset:0,g=e?b.axisX.offset:0,h=a.width()||c.quantity(b.width).value||0,i=a.height()||c.quantity(b.height).value||0,j=c.normalizePadding(b.chartPadding,d);h=Math.max(h,f+j.left+j.right),i=Math.max(i,g+j.top+j.bottom);var k={padding:j,width:function(){return this.x2-this.x1},height:function(){return this.y1-this.y2}};return e?("start"===b.axisX.position?(k.y2=j.top+g,k.y1=Math.max(i-j.bottom,k.y2+1)):(k.y2=j.top,k.y1=Math.max(i-j.bottom-g,k.y2+1)),"start"===b.axisY.position?(k.x1=j.left+f,k.x2=Math.max(h-j.right,k.x1+1)):(k.x1=j.left,k.x2=Math.max(h-j.right-f,k.x1+1))):(k.x1=j.left,k.x2=Math.max(h-j.right,k.x1+1),k.y2=j.top,k.y1=Math.max(i-j.bottom,k.y2+1)),k},c.createGrid=function(a,b,d,e,f,g,h,i){var j={};j[d.units.pos+"1"]=a,j[d.units.pos+"2"]=a,j[d.counterUnits.pos+"1"]=e,j[d.counterUnits.pos+"2"]=e+f;var k=g.elem("line",j,h.join(" "));i.emit("draw",c.extend({type:"grid",axis:d,index:b,group:g,element:k},j))},c.createGridBackground=function(a,b,c,d){var e=a.elem("rect",{x:b.x1,y:b.y2,width:b.width(),height:b.height()},c,!0);d.emit("draw",{type:"gridBackground",group:a,element:e})},c.createLabel=function(a,b,d,e,f,g,h,i,j,k,l){var m,n={};if(n[f.units.pos]=a+h[f.units.pos],n[f.counterUnits.pos]=h[f.counterUnits.pos],n[f.units.len]=b,n[f.counterUnits.len]=Math.max(0,g-10),k){var o='<span class="'+j.join(" ")+'" style="'+f.units.len+": "+Math.round(n[f.units.len])+"px; "+f.counterUnits.len+": "+Math.round(n[f.counterUnits.len])+'px">'+e[d]+"</span>";m=i.foreignObject(o,c.extend({style:"overflow: visible;"},n))}else m=i.elem("text",n,j.join(" ")).text(e[d]);l.emit("draw",c.extend({type:"label",axis:f,index:d,group:i,element:m,text:e[d]},n))},c.getSeriesOption=function(a,b,c){if(a.name&&b.series&&b.series[a.name]){var d=b.series[a.name];return d.hasOwnProperty(c)?d[c]:b[c]}return b[c]},c.optionsProvider=function(b,d,e){function f(b){var f=h;if(h=c.extend({},j),d)for(i=0;i<d.length;i++){var g=a.matchMedia(d[i][0]);g.matches&&(h=c.extend(h,d[i][1]))}e&&b&&e.emit("optionsChanged",{previousOptions:f,currentOptions:h})}function g(){k.forEach(function(a){a.removeListener(f)})}var h,i,j=c.extend({},b),k=[];if(!a.matchMedia)throw"window.matchMedia not found! Make sure you're using a polyfill.";if(d)for(i=0;i<d.length;i++){var l=a.matchMedia(d[i][0]);l.addListener(f),k.push(l)}return f(),{removeMediaQueryListeners:g,getCurrentOptions:function(){return c.extend({},h)}}},c.splitIntoSegments=function(a,b,d){var e={increasingX:!1,fillHoles:!1};d=c.extend({},e,d);for(var f=[],g=!0,h=0;h<a.length;h+=2)void 0===c.getMultiValue(b[h/2].value)?d.fillHoles||(g=!0):(d.increasingX&&h>=2&&a[h]<=a[h-2]&&(g=!0),g&&(f.push({pathCoordinates:[],valueData:[]}),g=!1),f[f.length-1].pathCoordinates.push(a[h],a[h+1]),f[f.length-1].valueData.push(b[h/2]));return f}}(window,document,a),function(a,b,c){"use strict";c.Interpolation={},c.Interpolation.none=function(a){var b={fillHoles:!1};return a=c.extend({},b,a),function(b,d){for(var e=new c.Svg.Path,f=!0,g=0;g<b.length;g+=2){var h=b[g],i=b[g+1],j=d[g/2];void 0!==c.getMultiValue(j.value)?(f?e.move(h,i,!1,j):e.line(h,i,!1,j),f=!1):a.fillHoles||(f=!0)}return e}},c.Interpolation.simple=function(a){var b={divisor:2,fillHoles:!1};a=c.extend({},b,a);var d=1/Math.max(1,a.divisor);return function(b,e){for(var f,g,h,i=new c.Svg.Path,j=0;j<b.length;j+=2){var k=b[j],l=b[j+1],m=(k-f)*d,n=e[j/2];void 0!==n.value?(void 0===h?i.move(k,l,!1,n):i.curve(f+m,g,k-m,l,k,l,!1,n),f=k,g=l,h=n):a.fillHoles||(f=k=h=void 0)}return i}},c.Interpolation.cardinal=function(a){var b={tension:1,fillHoles:!1};a=c.extend({},b,a);var d=Math.min(1,Math.max(0,a.tension)),e=1-d;return function f(b,g){var h=c.splitIntoSegments(b,g,{fillHoles:a.fillHoles});if(h.length){if(h.length>1){var i=[];return h.forEach(function(a){i.push(f(a.pathCoordinates,a.valueData))}),c.Svg.Path.join(i)}if(b=h[0].pathCoordinates,g=h[0].valueData,b.length<=4)return c.Interpolation.none()(b,g);for(var j,k=(new c.Svg.Path).move(b[0],b[1],!1,g[0]),l=0,m=b.length;m-2*!j>l;l+=2){var n=[{x:+b[l-2],y:+b[l-1]},{x:+b[l],y:+b[l+1]},{x:+b[l+2],y:+b[l+3]},{x:+b[l+4],y:+b[l+5]}];j?l?m-4===l?n[3]={x:+b[0],y:+b[1]}:m-2===l&&(n[2]={x:+b[0],y:+b[1]},n[3]={x:+b[2],y:+b[3]}):n[0]={x:+b[m-2],y:+b[m-1]}:m-4===l?n[3]=n[2]:l||(n[0]={x:+b[l],y:+b[l+1]}),k.curve(d*(-n[0].x+6*n[1].x+n[2].x)/6+e*n[2].x,d*(-n[0].y+6*n[1].y+n[2].y)/6+e*n[2].y,d*(n[1].x+6*n[2].x-n[3].x)/6+e*n[2].x,d*(n[1].y+6*n[2].y-n[3].y)/6+e*n[2].y,n[2].x,n[2].y,!1,g[(l+2)/2])}return k}return c.Interpolation.none()([])}},c.Interpolation.monotoneCubic=function(a){var b={fillHoles:!1};return a=c.extend({},b,a),function d(b,e){var f=c.splitIntoSegments(b,e,{fillHoles:a.fillHoles,increasingX:!0});if(f.length){if(f.length>1){var g=[];return f.forEach(function(a){g.push(d(a.pathCoordinates,a.valueData))}),c.Svg.Path.join(g)}if(b=f[0].pathCoordinates,e=f[0].valueData,b.length<=4)return c.Interpolation.none()(b,e);var h,i,j=[],k=[],l=b.length/2,m=[],n=[],o=[],p=[];for(h=0;h<l;h++)j[h]=b[2*h],k[h]=b[2*h+1];for(h=0;h<l-1;h++)o[h]=k[h+1]-k[h],p[h]=j[h+1]-j[h],n[h]=o[h]/p[h];for(m[0]=n[0],m[l-1]=n[l-2],h=1;h<l-1;h++)0===n[h]||0===n[h-1]||n[h-1]>0!=n[h]>0?m[h]=0:(m[h]=3*(p[h-1]+p[h])/((2*p[h]+p[h-1])/n[h-1]+(p[h]+2*p[h-1])/n[h]),isFinite(m[h])||(m[h]=0));for(i=(new c.Svg.Path).move(j[0],k[0],!1,e[0]),h=0;h<l-1;h++)i.curve(j[h]+p[h]/3,k[h]+m[h]*p[h]/3,j[h+1]-p[h]/3,k[h+1]-m[h+1]*p[h]/3,j[h+1],k[h+1],!1,e[h+1]);return i}return c.Interpolation.none()([])}},c.Interpolation.step=function(a){var b={postpone:!0,fillHoles:!1};return a=c.extend({},b,a),function(b,d){for(var e,f,g,h=new c.Svg.Path,i=0;i<b.length;i+=2){var j=b[i],k=b[i+1],l=d[i/2];void 0!==l.value?(void 0===g?h.move(j,k,!1,l):(a.postpone?h.line(j,f,!1,g):h.line(e,k,!1,l),h.line(j,k,!1,l)),e=j,f=k,g=l):a.fillHoles||(e=f=g=void 0)}return h}}}(window,document,a),function(a,b,c){"use strict";c.EventEmitter=function(){function a(a,b){d[a]=d[a]||[],d[a].push(b)}function b(a,b){d[a]&&(b?(d[a].splice(d[a].indexOf(b),1),0===d[a].length&&delete d[a]):delete d[a])}function c(a,b){d[a]&&d[a].forEach(function(a){a(b)}),d["*"]&&d["*"].forEach(function(c){c(a,b)})}var d=[];return{addEventHandler:a,removeEventHandler:b,emit:c}}}(window,document,a),function(a,b,c){"use strict";function d(a){var b=[];if(a.length)for(var c=0;c<a.length;c++)b.push(a[c]);return b}function e(a,b){var d=b||this.prototype||c.Class,e=Object.create(d);c.Class.cloneDefinitions(e,a);var f=function(){var a,b=e.constructor||function(){};return a=this===c?Object.create(e):this,b.apply(a,Array.prototype.slice.call(arguments,0)),a};return f.prototype=e,f["super"]=d,f.extend=this.extend,f}function f(){var a=d(arguments),b=a[0];return a.splice(1,a.length-1).forEach(function(a){Object.getOwnPropertyNames(a).forEach(function(c){delete b[c],Object.defineProperty(b,c,Object.getOwnPropertyDescriptor(a,c))})}),b}c.Class={extend:e,cloneDefinitions:f}}(window,document,a),function(a,b,c){"use strict";function d(a,b,d){return a&&(this.data=a||{},this.data.labels=this.data.labels||[],this.data.series=this.data.series||[],this.eventEmitter.emit("data",{type:"update",data:this.data})),b&&(this.options=c.extend({},d?this.options:this.defaultOptions,b),this.initializeTimeoutId||(this.optionsProvider.removeMediaQueryListeners(),this.optionsProvider=c.optionsProvider(this.options,this.responsiveOptions,this.eventEmitter))),this.initializeTimeoutId||this.createChart(this.optionsProvider.getCurrentOptions()),this}function e(){return this.initializeTimeoutId?a.clearTimeout(this.initializeTimeoutId):(a.removeEventListener("resize",this.resizeListener),this.optionsProvider.removeMediaQueryListeners()),this}function f(a,b){return this.eventEmitter.addEventHandler(a,b),this}function g(a,b){return this.eventEmitter.removeEventHandler(a,b),this}function h(){a.addEventListener("resize",this.resizeListener),this.optionsProvider=c.optionsProvider(this.options,this.responsiveOptions,this.eventEmitter),this.eventEmitter.addEventHandler("optionsChanged",function(){this.update()}.bind(this)),this.options.plugins&&this.options.plugins.forEach(function(a){a instanceof Array?a[0](this,a[1]):a(this)}.bind(this)),this.eventEmitter.emit("data",{type:"initial",data:this.data}),this.createChart(this.optionsProvider.getCurrentOptions()),this.initializeTimeoutId=void 0}function i(a,b,d,e,f){this.container=c.querySelector(a),this.data=b||{},this.data.labels=this.data.labels||[],this.data.series=this.data.series||[],this.defaultOptions=d,this.options=e,this.responsiveOptions=f,this.eventEmitter=c.EventEmitter(),this.supportsForeignObject=c.Svg.isSupported("Extensibility"),this.supportsAnimations=c.Svg.isSupported("AnimationEventsAttribute"),this.resizeListener=function(){this.update()}.bind(this),this.container&&(this.container.__chartist__&&this.container.__chartist__.detach(),this.container.__chartist__=this),this.initializeTimeoutId=setTimeout(h.bind(this),0)}c.Base=c.Class.extend({constructor:i,optionsProvider:void 0,container:void 0,svg:void 0,eventEmitter:void 0,createChart:function(){throw new Error("Base chart type can't be instantiated!")},update:d,detach:e,on:f,off:g,version:c.version,supportsForeignObject:!1})}(window,document,a),function(a,b,c){"use strict";function d(a,d,e,f,g){a instanceof Element?this._node=a:(this._node=b.createElementNS(c.namespaces.svg,a),"svg"===a&&this.attr({"xmlns:ct":c.namespaces.ct})),d&&this.attr(d),e&&this.addClass(e),f&&(g&&f._node.firstChild?f._node.insertBefore(this._node,f._node.firstChild):f._node.appendChild(this._node))}function e(a,b){return"string"==typeof a?b?this._node.getAttributeNS(b,a):this._node.getAttribute(a):(Object.keys(a).forEach(function(b){if(void 0!==a[b])if(b.indexOf(":")!==-1){var d=b.split(":");this._node.setAttributeNS(c.namespaces[d[0]],b,a[b])}else this._node.setAttribute(b,a[b])}.bind(this)),this)}function f(a,b,d,e){return new c.Svg(a,b,d,this,e)}function g(){return this._node.parentNode instanceof SVGElement?new c.Svg(this._node.parentNode):null}function h(){for(var a=this._node;"svg"!==a.nodeName;)a=a.parentNode;return new c.Svg(a)}function i(a){var b=this._node.querySelector(a);return b?new c.Svg(b):null}function j(a){var b=this._node.querySelectorAll(a);return b.length?new c.Svg.List(b):null}function k(){return this._node}function l(a,d,e,f){if("string"==typeof a){var g=b.createElement("div");g.innerHTML=a,a=g.firstChild}a.setAttribute("xmlns",c.namespaces.xmlns);var h=this.elem("foreignObject",d,e,f);return h._node.appendChild(a),h}function m(a){return this._node.appendChild(b.createTextNode(a)),this}function n(){for(;this._node.firstChild;)this._node.removeChild(this._node.firstChild);return this}function o(){return this._node.parentNode.removeChild(this._node),this.parent()}function p(a){return this._node.parentNode.replaceChild(a._node,this._node),a}function q(a,b){return b&&this._node.firstChild?this._node.insertBefore(a._node,this._node.firstChild):this._node.appendChild(a._node),this}function r(){return this._node.getAttribute("class")?this._node.getAttribute("class").trim().split(/\s+/):[]}function s(a){return this._node.setAttribute("class",this.classes(this._node).concat(a.trim().split(/\s+/)).filter(function(a,b,c){return c.indexOf(a)===b}).join(" ")),this}function t(a){var b=a.trim().split(/\s+/);return this._node.setAttribute("class",this.classes(this._node).filter(function(a){return b.indexOf(a)===-1}).join(" ")),this}function u(){return this._node.setAttribute("class",""),this}function v(){return this._node.getBoundingClientRect().height}function w(){return this._node.getBoundingClientRect().width}function x(a,b,d){return void 0===b&&(b=!0),Object.keys(a).forEach(function(e){function f(a,b){var f,g,h,i={};a.easing&&(h=a.easing instanceof Array?a.easing:c.Svg.Easing[a.easing],delete a.easing),a.begin=c.ensureUnit(a.begin,"ms"),a.dur=c.ensureUnit(a.dur,"ms"),h&&(a.calcMode="spline",a.keySplines=h.join(" "),a.keyTimes="0;1"),b&&(a.fill="freeze",i[e]=a.from,this.attr(i),g=c.quantity(a.begin||0).value,a.begin="indefinite"),f=this.elem("animate",c.extend({attributeName:e},a)),b&&setTimeout(function(){try{f._node.beginElement()}catch(b){i[e]=a.to,this.attr(i),f.remove()}}.bind(this),g),d&&f._node.addEventListener("beginEvent",function(){d.emit("animationBegin",{element:this,animate:f._node,params:a})}.bind(this)),f._node.addEventListener("endEvent",function(){d&&d.emit("animationEnd",{element:this,animate:f._node,params:a}),b&&(i[e]=a.to,this.attr(i),f.remove())}.bind(this))}a[e]instanceof Array?a[e].forEach(function(a){f.bind(this)(a,!1)}.bind(this)):f.bind(this)(a[e],b)}.bind(this)),this}function y(a){var b=this;this.svgElements=[];for(var d=0;d<a.length;d++)this.svgElements.push(new c.Svg(a[d]));Object.keys(c.Svg.prototype).filter(function(a){return["constructor","parent","querySelector","querySelectorAll","replace","append","classes","height","width"].indexOf(a)===-1}).forEach(function(a){b[a]=function(){var d=Array.prototype.slice.call(arguments,0);return b.svgElements.forEach(function(b){c.Svg.prototype[a].apply(b,d)}),b}})}c.Svg=c.Class.extend({constructor:d,attr:e,elem:f,parent:g,root:h,querySelector:i,querySelectorAll:j,getNode:k,foreignObject:l,text:m,empty:n,remove:o,replace:p,append:q,classes:r,addClass:s,removeClass:t,removeAllClasses:u,height:v,width:w,animate:x}),c.Svg.isSupported=function(a){return b.implementation.hasFeature("http://www.w3.org/TR/SVG11/feature#"+a,"1.1")};var z={easeInSine:[.47,0,.745,.715],easeOutSine:[.39,.575,.565,1],easeInOutSine:[.445,.05,.55,.95],easeInQuad:[.55,.085,.68,.53],easeOutQuad:[.25,.46,.45,.94],easeInOutQuad:[.455,.03,.515,.955],easeInCubic:[.55,.055,.675,.19],easeOutCubic:[.215,.61,.355,1],easeInOutCubic:[.645,.045,.355,1],easeInQuart:[.895,.03,.685,.22],easeOutQuart:[.165,.84,.44,1],easeInOutQuart:[.77,0,.175,1],easeInQuint:[.755,.05,.855,.06],easeOutQuint:[.23,1,.32,1],easeInOutQuint:[.86,0,.07,1],easeInExpo:[.95,.05,.795,.035],easeOutExpo:[.19,1,.22,1],easeInOutExpo:[1,0,0,1],easeInCirc:[.6,.04,.98,.335],easeOutCirc:[.075,.82,.165,1],easeInOutCirc:[.785,.135,.15,.86],easeInBack:[.6,-.28,.735,.045],easeOutBack:[.175,.885,.32,1.275],easeInOutBack:[.68,-.55,.265,1.55]};c.Svg.Easing=z,c.Svg.List=c.Class.extend({constructor:y})}(window,document,a),function(a,b,c){"use strict";function d(a,b,d,e,f,g){var h=c.extend({command:f?a.toLowerCase():a.toUpperCase()},b,g?{data:g}:{});d.splice(e,0,h)}function e(a,b){a.forEach(function(c,d){u[c.command.toLowerCase()].forEach(function(e,f){b(c,e,d,f,a)})})}function f(a,b){this.pathElements=[],this.pos=0,this.close=a,this.options=c.extend({},v,b)}function g(a){return void 0!==a?(this.pos=Math.max(0,Math.min(this.pathElements.length,a)),this):this.pos}function h(a){return this.pathElements.splice(this.pos,a),this}function i(a,b,c,e){return d("M",{x:+a,y:+b},this.pathElements,this.pos++,c,e),this}function j(a,b,c,e){return d("L",{x:+a,y:+b},this.pathElements,this.pos++,c,e),this}function k(a,b,c,e,f,g,h,i){return d("C",{x1:+a,y1:+b,x2:+c,y2:+e,x:+f,y:+g},this.pathElements,this.pos++,h,i),this}function l(a,b,c,e,f,g,h,i,j){return d("A",{rx:+a,ry:+b,xAr:+c,lAf:+e,sf:+f,x:+g,y:+h},this.pathElements,this.pos++,i,j),this}function m(a){var b=a.replace(/([A-Za-z])([0-9])/g,"$1 $2").replace(/([0-9])([A-Za-z])/g,"$1 $2").split(/[\s,]+/).reduce(function(a,b){return b.match(/[A-Za-z]/)&&a.push([]),a[a.length-1].push(b),a},[]);"Z"===b[b.length-1][0].toUpperCase()&&b.pop();var d=b.map(function(a){var b=a.shift(),d=u[b.toLowerCase()];return c.extend({command:b},d.reduce(function(b,c,d){return b[c]=+a[d],b},{}))}),e=[this.pos,0];return Array.prototype.push.apply(e,d),Array.prototype.splice.apply(this.pathElements,e),this.pos+=d.length,this}function n(){var a=Math.pow(10,this.options.accuracy);return this.pathElements.reduce(function(b,c){var d=u[c.command.toLowerCase()].map(function(b){return this.options.accuracy?Math.round(c[b]*a)/a:c[b]}.bind(this));return b+c.command+d.join(",")}.bind(this),"")+(this.close?"Z":"")}function o(a,b){return e(this.pathElements,function(c,d){c[d]*="x"===d[0]?a:b}),this}function p(a,b){return e(this.pathElements,function(c,d){c[d]+="x"===d[0]?a:b}),this}function q(a){return e(this.pathElements,function(b,c,d,e,f){var g=a(b,c,d,e,f);(g||0===g)&&(b[c]=g)}),this}function r(a){var b=new c.Svg.Path(a||this.close);return b.pos=this.pos,b.pathElements=this.pathElements.slice().map(function(a){return c.extend({},a)}),b.options=c.extend({},this.options),b}function s(a){var b=[new c.Svg.Path];return this.pathElements.forEach(function(d){d.command===a.toUpperCase()&&0!==b[b.length-1].pathElements.length&&b.push(new c.Svg.Path),b[b.length-1].pathElements.push(d)}),b}function t(a,b,d){for(var e=new c.Svg.Path(b,d),f=0;f<a.length;f++)for(var g=a[f],h=0;h<g.pathElements.length;h++)e.pathElements.push(g.pathElements[h]);return e}var u={m:["x","y"],l:["x","y"],c:["x1","y1","x2","y2","x","y"],a:["rx","ry","xAr","lAf","sf","x","y"]},v={accuracy:3};c.Svg.Path=c.Class.extend({constructor:f,position:g,remove:h,move:i,line:j,curve:k,arc:l,scale:o,translate:p,transform:q,parse:m,stringify:n,clone:r,splitByCommand:s}),c.Svg.Path.elementDescriptions=u,c.Svg.Path.join=t}(window,document,a),function(a,b,c){"use strict";function d(a,b,c,d){this.units=a,this.counterUnits=a===f.x?f.y:f.x,this.chartRect=b,this.axisLength=b[a.rectEnd]-b[a.rectStart],this.gridOffset=b[a.rectOffset],this.ticks=c,this.options=d}function e(a,b,d,e,f){var g=e["axis"+this.units.pos.toUpperCase()],h=this.ticks.map(this.projectValue.bind(this)),i=this.ticks.map(g.labelInterpolationFnc);h.forEach(function(j,k){var l,m={x:0,y:0};l=h[k+1]?h[k+1]-j:Math.max(this.axisLength-j,30),c.isFalseyButZero(i[k])&&""!==i[k]||("x"===this.units.pos?(j=this.chartRect.x1+j,m.x=e.axisX.labelOffset.x,"start"===e.axisX.position?m.y=this.chartRect.padding.top+e.axisX.labelOffset.y+(d?5:20):m.y=this.chartRect.y1+e.axisX.labelOffset.y+(d?5:20)):(j=this.chartRect.y1-j,m.y=e.axisY.labelOffset.y-(d?l:0),"start"===e.axisY.position?m.x=d?this.chartRect.padding.left+e.axisY.labelOffset.x:this.chartRect.x1-10:m.x=this.chartRect.x2+e.axisY.labelOffset.x+10),g.showGrid&&c.createGrid(j,k,this,this.gridOffset,this.chartRect[this.counterUnits.len](),a,[e.classNames.grid,e.classNames[this.units.dir]],f),g.showLabel&&c.createLabel(j,l,k,i,this,g.offset,m,b,[e.classNames.label,e.classNames[this.units.dir],"start"===g.position?e.classNames[g.position]:e.classNames.end],d,f))}.bind(this))}var f={x:{pos:"x",len:"width",dir:"horizontal",rectStart:"x1",rectEnd:"x2",rectOffset:"y2"},y:{pos:"y",len:"height",dir:"vertical",rectStart:"y2",rectEnd:"y1",rectOffset:"x1"}};c.Axis=c.Class.extend({constructor:d,createGridAndLabels:e,projectValue:function(a,b,c){throw new Error("Base axis can't be instantiated!")}}),c.Axis.units=f}(window,document,a),function(a,b,c){"use strict";function d(a,b,d,e){var f=e.highLow||c.getHighLow(b,e,a.pos);this.bounds=c.getBounds(d[a.rectEnd]-d[a.rectStart],f,e.scaleMinSpace||20,e.onlyInteger),this.range={min:this.bounds.min,max:this.bounds.max},c.AutoScaleAxis["super"].constructor.call(this,a,d,this.bounds.values,e)}function e(a){return this.axisLength*(+c.getMultiValue(a,this.units.pos)-this.bounds.min)/this.bounds.range}c.AutoScaleAxis=c.Axis.extend({constructor:d,projectValue:e})}(window,document,a),function(a,b,c){"use strict";function d(a,b,d,e){var f=e.highLow||c.getHighLow(b,e,a.pos);this.divisor=e.divisor||1,this.ticks=e.ticks||c.times(this.divisor).map(function(a,b){return f.low+(f.high-f.low)/this.divisor*b}.bind(this)),this.ticks.sort(function(a,b){return a-b}),this.range={min:f.low,max:f.high},c.FixedScaleAxis["super"].constructor.call(this,a,d,this.ticks,e),this.stepLength=this.axisLength/this.divisor}function e(a){return this.axisLength*(+c.getMultiValue(a,this.units.pos)-this.range.min)/(this.range.max-this.range.min)}c.FixedScaleAxis=c.Axis.extend({constructor:d,projectValue:e})}(window,document,a),function(a,b,c){"use strict";function d(a,b,d,e){c.StepAxis["super"].constructor.call(this,a,d,e.ticks,e);var f=Math.max(1,e.ticks.length-(e.stretch?1:0));this.stepLength=this.axisLength/f}function e(a,b){return this.stepLength*b}c.StepAxis=c.Axis.extend({constructor:d,projectValue:e})}(window,document,a),function(a,b,c){"use strict";function d(a){var b=c.normalizeData(this.data,a.reverseData,!0);this.svg=c.createSvg(this.container,a.width,a.height,a.classNames.chart);var d,e,g=this.svg.elem("g").addClass(a.classNames.gridGroup),h=this.svg.elem("g"),i=this.svg.elem("g").addClass(a.classNames.labelGroup),j=c.createChartRect(this.svg,a,f.padding);d=void 0===a.axisX.type?new c.StepAxis(c.Axis.units.x,b.normalized.series,j,c.extend({},a.axisX,{ticks:b.normalized.labels,stretch:a.fullWidth})):a.axisX.type.call(c,c.Axis.units.x,b.normalized.series,j,a.axisX),e=void 0===a.axisY.type?new c.AutoScaleAxis(c.Axis.units.y,b.normalized.series,j,c.extend({},a.axisY,{high:c.isNumeric(a.high)?a.high:a.axisY.high,low:c.isNumeric(a.low)?a.low:a.axisY.low})):a.axisY.type.call(c,c.Axis.units.y,b.normalized.series,j,a.axisY),d.createGridAndLabels(g,i,this.supportsForeignObject,a,this.eventEmitter),e.createGridAndLabels(g,i,this.supportsForeignObject,a,this.eventEmitter),a.showGridBackground&&c.createGridBackground(g,j,a.classNames.gridBackground,this.eventEmitter),b.raw.series.forEach(function(f,g){var i=h.elem("g");i.attr({"ct:series-name":f.name,"ct:meta":c.serialize(f.meta)}),i.addClass([a.classNames.series,f.className||a.classNames.series+"-"+c.alphaNumerate(g)].join(" "));var k=[],l=[];b.normalized.series[g].forEach(function(a,h){var i={x:j.x1+d.projectValue(a,h,b.normalized.series[g]),y:j.y1-e.projectValue(a,h,b.normalized.series[g])};k.push(i.x,i.y),l.push({value:a,valueIndex:h,meta:c.getMetaData(f,h)})}.bind(this));var m={lineSmooth:c.getSeriesOption(f,a,"lineSmooth"),showPoint:c.getSeriesOption(f,a,"showPoint"),showLine:c.getSeriesOption(f,a,"showLine"),showArea:c.getSeriesOption(f,a,"showArea"),areaBase:c.getSeriesOption(f,a,"areaBase")},n="function"==typeof m.lineSmooth?m.lineSmooth:m.lineSmooth?c.Interpolation.monotoneCubic():c.Interpolation.none(),o=n(k,l);if(m.showPoint&&o.pathElements.forEach(function(b){var h=i.elem("line",{x1:b.x,y1:b.y,x2:b.x+.01,y2:b.y},a.classNames.point).attr({"ct:value":[b.data.value.x,b.data.value.y].filter(c.isNumeric).join(","),"ct:meta":c.serialize(b.data.meta)});this.eventEmitter.emit("draw",{type:"point",value:b.data.value,index:b.data.valueIndex,meta:b.data.meta,series:f,seriesIndex:g,axisX:d,axisY:e,group:i,element:h,x:b.x,y:b.y})}.bind(this)),m.showLine){var p=i.elem("path",{d:o.stringify()},a.classNames.line,!0);this.eventEmitter.emit("draw",{type:"line",values:b.normalized.series[g],path:o.clone(),chartRect:j,index:g,series:f,seriesIndex:g,seriesMeta:f.meta,axisX:d,axisY:e,group:i,element:p})}if(m.showArea&&e.range){var q=Math.max(Math.min(m.areaBase,e.range.max),e.range.min),r=j.y1-e.projectValue(q);o.splitByCommand("M").filter(function(a){return a.pathElements.length>1}).map(function(a){var b=a.pathElements[0],c=a.pathElements[a.pathElements.length-1];return a.clone(!0).position(0).remove(1).move(b.x,r).line(b.x,b.y).position(a.pathElements.length+1).line(c.x,r)}).forEach(function(c){var h=i.elem("path",{d:c.stringify()},a.classNames.area,!0);this.eventEmitter.emit("draw",{type:"area",values:b.normalized.series[g],path:c.clone(),series:f,seriesIndex:g,axisX:d,axisY:e,chartRect:j,index:g,group:i,element:h})}.bind(this))}}.bind(this)),this.eventEmitter.emit("created",{bounds:e.bounds,chartRect:j,axisX:d,axisY:e,svg:this.svg,options:a})}function e(a,b,d,e){c.Line["super"].constructor.call(this,a,b,f,c.extend({},f,d),e)}var f={axisX:{offset:30,position:"end",labelOffset:{x:0,y:0},showLabel:!0,showGrid:!0,labelInterpolationFnc:c.noop,type:void 0},axisY:{offset:40,position:"start",labelOffset:{x:0,y:0},showLabel:!0,showGrid:!0,labelInterpolationFnc:c.noop,type:void 0,scaleMinSpace:20,onlyInteger:!1},width:void 0,height:void 0,showLine:!0,showPoint:!0,showArea:!1,areaBase:0,lineSmooth:!0,showGridBackground:!1,low:void 0,high:void 0,chartPadding:{top:15,right:15,bottom:5,left:10},fullWidth:!1,reverseData:!1,classNames:{chart:"ct-chart-line",label:"ct-label",labelGroup:"ct-labels",series:"ct-series",line:"ct-line",point:"ct-point",area:"ct-area",grid:"ct-grid",gridGroup:"ct-grids",gridBackground:"ct-grid-background",vertical:"ct-vertical",horizontal:"ct-horizontal",start:"ct-start",end:"ct-end"}};c.Line=c.Base.extend({constructor:e,createChart:d})}(window,document,a),function(a,b,c){"use strict";function d(a){var b,d;a.distributeSeries?(b=c.normalizeData(this.data,a.reverseData,a.horizontalBars?"x":"y"),b.normalized.series=b.normalized.series.map(function(a){return[a]})):b=c.normalizeData(this.data,a.reverseData,a.horizontalBars?"x":"y"),this.svg=c.createSvg(this.container,a.width,a.height,a.classNames.chart+(a.horizontalBars?" "+a.classNames.horizontalBars:""));var e=this.svg.elem("g").addClass(a.classNames.gridGroup),g=this.svg.elem("g"),h=this.svg.elem("g").addClass(a.classNames.labelGroup);if(a.stackBars&&0!==b.normalized.series.length){var i=c.serialMap(b.normalized.series,function(){return Array.prototype.slice.call(arguments).map(function(a){
    return a}).reduce(function(a,b){return{x:a.x+(b&&b.x)||0,y:a.y+(b&&b.y)||0}},{x:0,y:0})});d=c.getHighLow([i],a,a.horizontalBars?"x":"y")}else d=c.getHighLow(b.normalized.series,a,a.horizontalBars?"x":"y");d.high=+a.high||(0===a.high?0:d.high),d.low=+a.low||(0===a.low?0:d.low);var j,k,l,m,n,o=c.createChartRect(this.svg,a,f.padding);k=a.distributeSeries&&a.stackBars?b.normalized.labels.slice(0,1):b.normalized.labels,a.horizontalBars?(j=m=void 0===a.axisX.type?new c.AutoScaleAxis(c.Axis.units.x,b.normalized.series,o,c.extend({},a.axisX,{highLow:d,referenceValue:0})):a.axisX.type.call(c,c.Axis.units.x,b.normalized.series,o,c.extend({},a.axisX,{highLow:d,referenceValue:0})),l=n=void 0===a.axisY.type?new c.StepAxis(c.Axis.units.y,b.normalized.series,o,{ticks:k}):a.axisY.type.call(c,c.Axis.units.y,b.normalized.series,o,a.axisY)):(l=m=void 0===a.axisX.type?new c.StepAxis(c.Axis.units.x,b.normalized.series,o,{ticks:k}):a.axisX.type.call(c,c.Axis.units.x,b.normalized.series,o,a.axisX),j=n=void 0===a.axisY.type?new c.AutoScaleAxis(c.Axis.units.y,b.normalized.series,o,c.extend({},a.axisY,{highLow:d,referenceValue:0})):a.axisY.type.call(c,c.Axis.units.y,b.normalized.series,o,c.extend({},a.axisY,{highLow:d,referenceValue:0})));var p=a.horizontalBars?o.x1+j.projectValue(0):o.y1-j.projectValue(0),q=[];l.createGridAndLabels(e,h,this.supportsForeignObject,a,this.eventEmitter),j.createGridAndLabels(e,h,this.supportsForeignObject,a,this.eventEmitter),a.showGridBackground&&c.createGridBackground(e,o,a.classNames.gridBackground,this.eventEmitter),b.raw.series.forEach(function(d,e){var f,h,i=e-(b.raw.series.length-1)/2;f=a.distributeSeries&&!a.stackBars?l.axisLength/b.normalized.series.length/2:a.distributeSeries&&a.stackBars?l.axisLength/2:l.axisLength/b.normalized.series[e].length/2,h=g.elem("g"),h.attr({"ct:series-name":d.name,"ct:meta":c.serialize(d.meta)}),h.addClass([a.classNames.series,d.className||a.classNames.series+"-"+c.alphaNumerate(e)].join(" ")),b.normalized.series[e].forEach(function(g,k){var r,s,t,u;if(u=a.distributeSeries&&!a.stackBars?e:a.distributeSeries&&a.stackBars?0:k,r=a.horizontalBars?{x:o.x1+j.projectValue(g&&g.x?g.x:0,k,b.normalized.series[e]),y:o.y1-l.projectValue(g&&g.y?g.y:0,u,b.normalized.series[e])}:{x:o.x1+l.projectValue(g&&g.x?g.x:0,u,b.normalized.series[e]),y:o.y1-j.projectValue(g&&g.y?g.y:0,k,b.normalized.series[e])},l instanceof c.StepAxis&&(l.options.stretch||(r[l.units.pos]+=f*(a.horizontalBars?-1:1)),r[l.units.pos]+=a.stackBars||a.distributeSeries?0:i*a.seriesBarDistance*(a.horizontalBars?-1:1)),t=q[k]||p,q[k]=t-(p-r[l.counterUnits.pos]),void 0!==g){var v={};v[l.units.pos+"1"]=r[l.units.pos],v[l.units.pos+"2"]=r[l.units.pos],!a.stackBars||"accumulate"!==a.stackMode&&a.stackMode?(v[l.counterUnits.pos+"1"]=p,v[l.counterUnits.pos+"2"]=r[l.counterUnits.pos]):(v[l.counterUnits.pos+"1"]=t,v[l.counterUnits.pos+"2"]=q[k]),v.x1=Math.min(Math.max(v.x1,o.x1),o.x2),v.x2=Math.min(Math.max(v.x2,o.x1),o.x2),v.y1=Math.min(Math.max(v.y1,o.y2),o.y1),v.y2=Math.min(Math.max(v.y2,o.y2),o.y1);var w=c.getMetaData(d,k);s=h.elem("line",v,a.classNames.bar).attr({"ct:value":[g.x,g.y].filter(c.isNumeric).join(","),"ct:meta":c.serialize(w)}),this.eventEmitter.emit("draw",c.extend({type:"bar",value:g,index:k,meta:w,series:d,seriesIndex:e,axisX:m,axisY:n,chartRect:o,group:h,element:s},v))}}.bind(this))}.bind(this)),this.eventEmitter.emit("created",{bounds:j.bounds,chartRect:o,axisX:m,axisY:n,svg:this.svg,options:a})}function e(a,b,d,e){c.Bar["super"].constructor.call(this,a,b,f,c.extend({},f,d),e)}var f={axisX:{offset:30,position:"end",labelOffset:{x:0,y:0},showLabel:!0,showGrid:!0,labelInterpolationFnc:c.noop,scaleMinSpace:30,onlyInteger:!1},axisY:{offset:40,position:"start",labelOffset:{x:0,y:0},showLabel:!0,showGrid:!0,labelInterpolationFnc:c.noop,scaleMinSpace:20,onlyInteger:!1},width:void 0,height:void 0,high:void 0,low:void 0,referenceValue:0,chartPadding:{top:15,right:15,bottom:5,left:10},seriesBarDistance:15,stackBars:!1,stackMode:"accumulate",horizontalBars:!1,distributeSeries:!1,reverseData:!1,showGridBackground:!1,classNames:{chart:"ct-chart-bar",horizontalBars:"ct-horizontal-bars",label:"ct-label",labelGroup:"ct-labels",series:"ct-series",bar:"ct-bar",grid:"ct-grid",gridGroup:"ct-grids",gridBackground:"ct-grid-background",vertical:"ct-vertical",horizontal:"ct-horizontal",start:"ct-start",end:"ct-end"}};c.Bar=c.Base.extend({constructor:e,createChart:d})}(window,document,a),function(a,b,c){"use strict";function d(a,b,c){var d=b.x>a.x;return d&&"explode"===c||!d&&"implode"===c?"start":d&&"implode"===c||!d&&"explode"===c?"end":"middle"}function e(a){var b,e,f,h,i,j=c.normalizeData(this.data),k=[],l=a.startAngle;this.svg=c.createSvg(this.container,a.width,a.height,a.donut?a.classNames.chartDonut:a.classNames.chartPie),e=c.createChartRect(this.svg,a,g.padding),f=Math.min(e.width()/2,e.height()/2),i=a.total||j.normalized.series.reduce(function(a,b){return a+b},0);var m=c.quantity(a.donutWidth);"%"===m.unit&&(m.value*=f/100),f-=a.donut?m.value/2:0,h="outside"===a.labelPosition||a.donut?f:"center"===a.labelPosition?0:f/2,h+=a.labelOffset;var n={x:e.x1+e.width()/2,y:e.y2+e.height()/2},o=1===j.raw.series.filter(function(a){return a.hasOwnProperty("value")?0!==a.value:0!==a}).length;j.raw.series.forEach(function(a,b){k[b]=this.svg.elem("g",null,null)}.bind(this)),a.showLabel&&(b=this.svg.elem("g",null,null)),j.raw.series.forEach(function(e,g){if(0!==j.normalized.series[g]||!a.ignoreEmptyValues){k[g].attr({"ct:series-name":e.name}),k[g].addClass([a.classNames.series,e.className||a.classNames.series+"-"+c.alphaNumerate(g)].join(" "));var p=i>0?l+j.normalized.series[g]/i*360:0,q=Math.max(0,l-(0===g||o?0:.2));p-q>=359.99&&(p=q+359.99);var r=c.polarToCartesian(n.x,n.y,f,q),s=c.polarToCartesian(n.x,n.y,f,p),t=new c.Svg.Path((!a.donut)).move(s.x,s.y).arc(f,f,0,p-l>180,0,r.x,r.y);a.donut||t.line(n.x,n.y);var u=k[g].elem("path",{d:t.stringify()},a.donut?a.classNames.sliceDonut:a.classNames.slicePie);if(u.attr({"ct:value":j.normalized.series[g],"ct:meta":c.serialize(e.meta)}),a.donut&&u.attr({style:"stroke-width: "+m.value+"px"}),this.eventEmitter.emit("draw",{type:"slice",value:j.normalized.series[g],totalDataSum:i,index:g,meta:e.meta,series:e,group:k[g],element:u,path:t.clone(),center:n,radius:f,startAngle:l,endAngle:p}),a.showLabel){var v;v=1===j.raw.series.length?{x:n.x,y:n.y}:c.polarToCartesian(n.x,n.y,h,l+(p-l)/2);var w;w=j.normalized.labels&&!c.isFalseyButZero(j.normalized.labels[g])?j.normalized.labels[g]:j.normalized.series[g];var x=a.labelInterpolationFnc(w,g);if(x||0===x){var y=b.elem("text",{dx:v.x,dy:v.y,"text-anchor":d(n,v,a.labelDirection)},a.classNames.label).text(""+x);this.eventEmitter.emit("draw",{type:"label",index:g,group:b,element:y,text:""+x,x:v.x,y:v.y})}}l=p}}.bind(this)),this.eventEmitter.emit("created",{chartRect:e,svg:this.svg,options:a})}function f(a,b,d,e){c.Pie["super"].constructor.call(this,a,b,g,c.extend({},g,d),e)}var g={width:void 0,height:void 0,chartPadding:5,classNames:{chartPie:"ct-chart-pie",chartDonut:"ct-chart-donut",series:"ct-series",slicePie:"ct-slice-pie",sliceDonut:"ct-slice-donut",label:"ct-label"},startAngle:0,total:void 0,donut:!1,donutWidth:60,showLabel:!0,labelOffset:0,labelPosition:"inside",labelInterpolationFnc:c.noop,labelDirection:"neutral",reverseData:!1,ignoreEmptyValues:!1};c.Pie=c.Base.extend({constructor:f,createChart:e,determineAnchorPosition:d})}(window,document,a),a});


)___";
std::string jsChartistPlugin = R"___(
/**
 * Chartist.js plugin to display a data label on top of the points in a line chart.
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Markus Padourek
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. **/

/* global Chartist */
(function (window, document, Chartist) {
    'use strict';

    var defaultOptions = {
        currency: undefined,
        currencyFormatCallback: undefined,
        tooltipOffset: {
            x: 0,
            y: -20
        },
        anchorToPoint: false,
        appendToBody: false,
        class: undefined,
        pointClass: 'ct-point'
    };

    Chartist.plugins = Chartist.plugins || {};
    Chartist.plugins.tooltip = function (options) {
        options = Chartist.extend({}, defaultOptions, options);

        return function tooltip(chart) {
            var tooltipSelector = options.pointClass;
            if (chart instanceof Chartist.Bar) {
                tooltipSelector = 'ct-bar';
            } else if (chart instanceof Chartist.Pie) {
                // Added support for donut graph
                if (chart.options.donut) {
                    tooltipSelector = 'ct-slice-donut';
                } else {
                    tooltipSelector = 'ct-slice-pie';
                }
            }

            var $chart = chart.container;
            var $toolTip = $chart.querySelector('.chartist-tooltip');
            if (!$toolTip) {
                $toolTip = document.createElement('div');
                $toolTip.className = (!options.class) ? 'chartist-tooltip' : 'chartist-tooltip ' + options.class;
                if (!options.appendToBody) {
                    $chart.appendChild($toolTip);
                } else {
                    document.body.appendChild($toolTip);
                }
            }
            var height = $toolTip.offsetHeight;
            var width = $toolTip.offsetWidth;

            hide($toolTip);

            function on(event, selector, callback) {
                $chart.addEventListener(event, function (e) {
                    if (!selector || hasClass(e.target, selector))
                        callback(e);
                });
            }

            on('mouseover', tooltipSelector, function (event) {
                var $point = event.target;
                var tooltipText = '';

                var isPieChart = (chart instanceof Chartist.Pie) ? $point : $point.parentNode;
                var seriesName = (isPieChart) ? $point.parentNode.getAttribute('ct:meta') || $point.parentNode.getAttribute('ct:series-name') : '';
                var meta = $point.getAttribute('ct:meta') || seriesName || '';
                var hasMeta = !!meta;
                var value = $point.getAttribute('ct:value');
                if (options.transformTooltipTextFnc && typeof options.transformTooltipTextFnc === 'function') {
                    value = options.transformTooltipTextFnc(value);
                }

                if (options.tooltipFnc && typeof options.tooltipFnc === 'function') {
                    tooltipText = options.tooltipFnc(meta, value);
                } else {
                    if (options.metaIsHTML) {
                        var txt = document.createElement('textarea');
                        txt.innerHTML = meta;
                        meta = txt.value;
                    }

                    meta = '<span class="chartist-tooltip-meta">' + meta + '</span>';

                    if (hasMeta) {
                        tooltipText += meta + '<br>';
                    } else {
                        // For Pie Charts also take the labels into account
                        // Could add support for more charts here as well!
                        if (chart instanceof Chartist.Pie) {
                            var label = next($point, 'ct-label');
                            if (label) {
                                tooltipText += text(label) + '<br>';
                            }
                        }
                    }

                    if (value) {
                        if (value % 1 === 0) {
                            value = '<span class="chartist-tooltip-value">' + minify_numbers(value) + '</span>';
                            tooltipText += value;
                        } else {
                            value = '<span class="chartist-tooltip-value">' + humanise_time(parseFloat(value)) + '</span>';
                            tooltipText += value;
                        }
                    }
                }

                if(tooltipText) {
                    $toolTip.innerHTML = tooltipText;
                    setPosition(event);
                    show($toolTip);

                    // Remember height and width to avoid wrong position in IE
                    height = $toolTip.offsetHeight;
                    width = $toolTip.offsetWidth;
                }
            });

            on('mouseout', tooltipSelector, function () {
                hide($toolTip);
            });

            on('mousemove', null, function (event) {
                if (false === options.anchorToPoint)
                    setPosition(event);
            });

            function setPosition(event) {
                height = height || $toolTip.offsetHeight;
                width = width || $toolTip.offsetWidth;
                var offsetX = - width / 2 + options.tooltipOffset.x;
                var offsetY = - height + options.tooltipOffset.y;
                var anchorX, anchorY;

                if (!options.appendToBody) {
                    var scrollTop = (window.pageYOffset !== undefined) ? window.pageYOffset : (document.documentElement || document.body.parentNode || document.body).scrollTop;
                    var top = event.pageY - window.pageYOffset + scrollTop ;
                    var left = event.pageX- window.pageXOffset;

                    if (true === options.anchorToPoint && event.target.x2 && event.target.y2) {
                        anchorX = parseInt(event.target.x2.baseVal.value);
                        anchorY = parseInt(event.target.y2.baseVal.value);
                    }

                    $toolTip.style.top = (anchorY || top) + offsetY + 'px';
                    $toolTip.style.left = (anchorX || left) + offsetX + 'px';
                } else {
                    $toolTip.style.top = event.pageY + offsetY + 'px';
                    $toolTip.style.left = event.pageX + offsetX + 'px';
                }
            }
        }
    };

    function show(element) {
        if(!hasClass(element, 'tooltip-show')) {
            element.className = element.className + ' tooltip-show';
        }
    }

    function hide(element) {
        var regex = new RegExp('tooltip-show' + '\\s*', 'gi');
        element.className = element.className.replace(regex, '').trim();
    }

    function hasClass(element, className) {
        return (' ' + element.getAttribute('class') + ' ').indexOf(' ' + className + ' ') > -1;
    }

    function next(element, className) {
        do {
            element = element.nextSibling;
        } while (element && !hasClass(element, className));
        return element;
    }

    function text(element) {
        return element.innerText || element.textContent;
    }

} (window, document, Chartist));

)___";
std::string jsMain = R"___(
function changeSelectedRel(id) {
    selected.rel = id;
    highlightRow();
    genRulesOfRelations();
}

function changeSelectedRul(id) {
    selected.rul = id;
    highlightRow();
    genRulVer();
    genAtomVer();
}

function highlightRow() {
    var i;
    for (i=0;i<document.getElementsByClassName("rel_row").length;i++) {
        document.getElementsByClassName("rel_row")[i].style.background = "rgba(255,255,0,0)";
    }
    for (i=0;i<document.getElementsByClassName("rul_row").length;i++) {
        document.getElementsByClassName("rul_row")[i].style.background = "rgba(255,255,0,0)";
    }
    if (selected.rul) {
        document.getElementById(selected.rul).style.background = "rgba(255,255,0,0.2)";
    }
    if (selected.rel) {
        document.getElementById(selected.rel).style.background = "rgba(255,255,0,0.2)";
    }
}


function graphRel() {
    if (!selected.rel) {
        alert("please select a relation to graph");
        return;
    }

    graph_vals.labels = [];
    graph_vals.tot_t = [];
    graph_vals.tuples = [];
    for (j = 0; j < data.rel[selected.rel][9].tot_t.length; j++) {
        graph_vals.labels.push(j.toString());
        graph_vals.tot_t.push(
            data.rel[selected.rel][9].tot_t[j]
        );
        graph_vals.tuples.push(
            data.rel[selected.rel][9].tuples[j]
        )
    }

    document.getElementById('chart_tab').click();
    drawGraph();
}

function graphIterRul() {
    if (!selected.rul || selected.rul[0]!='C') {
        alert("Please select a recursive rule (ID starts with C) to graph.");
        return;
    }

    came_from = "rul";

    graph_vals.labels = [];
    graph_vals.tot_t = [];
    graph_vals.tuples = [];
    for (j = 0; j < data.rul[selected.rul][9].tot_t.length; j++) {
        graph_vals.labels.push(j.toString());
        graph_vals.tot_t.push(
            data.rul[selected.rul][9].tot_t[j]
        );
        graph_vals.tuples.push(
            data.rul[selected.rul][9].tuples[j]
        )
    }

    document.getElementById('chart_tab').click();
    drawGraph();
}


function graphUsages() {
    graph_vals.labels = [];
    graph_vals.cpu = [];
    graph_vals.rss = [];
    var interval = Math.ceil(data.usage.length / 8);
    for (j = 0; j < data.usage.length; j++) {
        graph_vals.labels.push(
            j % interval == 0 ? data.usage[j][0] : "");
        graph_vals.cpu.push(
            {meta: data.usage[j][4], value: (data.usage[j][1] + data.usage[j][2]).toString()});
        graph_vals.rss.push(
            {meta: data.usage[j][4], value: data.usage[j][3].toString()});
    }

    var options = {
        height: "calc((100vh - 167px) / 2)",
        axisY: {
            labelInterpolationFnc: function (value) {
                return value.toFixed(0) + '%';
            }
        },
        axisX: {
            labelInterpolationFnc: function (value) {
                if (!value) {
                    return "";
                }
                return humanise_time(value);
            }
        },
        plugins: [Chartist.plugins.tooltip({tooltipFnc: function (meta, value) {
                value = Number(value);
                return meta + '<br/>' + value.toFixed(0) + '%';}})]
    };

    new Chartist.Bar(".ct-chart-cpu", {
        labels: graph_vals.labels,
        series: [graph_vals.cpu],
    }, options);

    options.axisY = {
        labelInterpolationFnc: function (value) {
            return minify_memory(value);
        },
    };

    options.plugins = [Chartist.plugins.tooltip({tooltipFnc: function(meta, value) {
                return meta + '<br/>' + minify_memory(value);}})]
    new Chartist.Bar(".ct-chart-rss", {
        labels: graph_vals.labels,
        series: [graph_vals.rss],
    }, options)
}

function drawGraph() {
    var options = {
        height: "calc((100vh - 167px) / 2)",
        axisY: {
            labelInterpolationFnc: function (value) {
                return humanise_time(value);
            }
        },
        axisX: {
            labelInterpolationFnc: function (value) {
                var n = Math.floor(graph_vals.labels.length/15);
                if (n>1) {
                    if (value%n == 0) {
                        return value;
                    }
                    return null;
                }
                return value;
            }
        },
        plugins: [Chartist.plugins.tooltip()]
    };

    new Chartist.Bar(".ct-chart1", {
        labels: graph_vals.labels,
        series: [graph_vals.tot_t],
    }, options);

    options.axisY = {
        labelInterpolationFnc: function (value) {
            return minify_numbers(value);
        }
    };

    new Chartist.Bar(".ct-chart2", {
        labels: graph_vals.labels,
        series: [graph_vals.tuples],
    }, options)
}


function changeTab(event, change_to) {
    if (change_to === "Chart") {
        document.getElementById("chart-tab").style.display = "block";
    } else {
        document.getElementById("chart-tab").style.display = "none";
    }
    var c, d, e;
    d = document.getElementsByClassName("tabcontent");
    for (c = 0; c < d.length; c++) {
        d[c].style.display = "none";
    }

    e = document.getElementsByClassName("tablinks");
    for (c = 0; c < e.length; c++) {
        e[c].className = e[c].className.replace(" active", "");
    }

    document.getElementById(change_to).style.display = "block";
    event.currentTarget.className += " active"
}


function toggle_precision() {
    precision=!precision;
    flip_table_values(document.getElementById("Rel_table"));
    flip_table_values(document.getElementById("Rul_table"));
    flip_table_values(document.getElementById("rulesofrel_table"));
    flip_table_values(document.getElementById("rulvertable"));
}

function flip_table_values(table) {
    var i,j,cell;
    for (i in table.rows) {
        if (!table.rows.hasOwnProperty(i)) continue;
        for (j in table.rows[i].cells) {
            if (! table.rows[i].cells.hasOwnProperty(j)) continue;
            cell = table.rows[i].cells[j];
            if (cell.className === "time_cell") {
                val = cell.getAttribute('data-sort');
                cell.innerHTML = humanise_time(parseFloat(val));
            } else if (cell.className === "int_cell") {
                val = cell.getAttribute('data-sort');
                cell.innerHTML = minify_numbers(parseInt(val));
            }
        }
    }
}

function create_cell(type, value, perc_total) {
    cell = document.createElement("td");

    if (type === "text") {
        cell.className = "text_cell";
        text_span = document.createElement("span");
        text_span.innerHTML = value;
        cell.appendChild(text_span);
    } else if (type === "code_loc") {
        cell.className = "text_cell";
        text_span = document.createElement("span");
        text_span.innerHTML = value;
        cell.appendChild(text_span);
        if (data.hasOwnProperty("code"))
            text_span.onclick = function() {view_code_snippet(value);}
    } else if (type === "id") {
        cell.innerHTML = value;
    } else if (type === "time") {
        cell.innerHTML = humanise_time(value);
        cell.setAttribute('data-sort', value);
        cell.className = "time_cell";
    } else if (type === "int") {
        cell.innerHTML = minify_numbers(value);
        cell.setAttribute('data-sort', value);
        cell.className = "int_cell";
    } else if (type === "perc") {
        div = document.createElement("div");
        div.className = "perc_time";
        if (perc_total == 0) {
            div.style.width = "0%";
            div.innerHTML = "0";
        } else if (isNaN(value)) {
            div.style.width = "0%";
            div.innerHTML = "NaN";
        } else {
            div.style.width = parseFloat(value) / perc_total * 100 + "%";
            div.innerHTML = clean_percentages(parseFloat(value) / perc_total * 100);
        }
        cell.appendChild(div);
    }
    return cell;
}


function generate_table(data_format,body_id,data_key) {
    var item,i;
    var perc_totals = [];

    for (i in data_format) {
        if (!data_format.hasOwnProperty(i)) continue;
        if (data_format[i][0] === "perc") {
            perc_totals.push([data_format[i][1],data_format[i][2],0]);
        }
    }

    for (item in data[data_key]) {
        if (data[data_key].hasOwnProperty(item)) {
            for (i in perc_totals) {
                if (!isNaN(data[data_key][item][perc_totals[i][1]])) {
                    perc_totals[i][2] += data[data_key][item][perc_totals[i][1]];
                }
            }
        }
    }


    table_body = document.getElementById(body_id);
    table_body.innerHTML = "";
    for (item in data[data_key]) {
        if (data[data_key].hasOwnProperty(item)) {
            row = document.createElement("tr");
            row.id = item;

            if (data_key === "rel") {
                row.className = "rel_row";
                row.onclick = function () {
                    changeSelectedRel(this.id);
                };
            } else if (data_key === "rul") {
                row.className = "rul_row";
                row.onclick = function () {
                    changeSelectedRul(this.id);
                };
            }
            perc_counter = 0;
            for (i in data_format) {
                if (!data_format.hasOwnProperty(i)) continue;
                if (data_format[i][0] === "perc") {
                    cell = create_cell(data_format[i][0], data[data_key][item][data_format[i][2]], perc_totals[perc_counter++][2]);
                } else {
                    cell = create_cell(data_format[i][0], data[data_key][item][data_format[i][1]]);
                }
                row.appendChild(cell);
            }
            table_body.appendChild(row);
        }
    }
}

function gen_rel_table() {
    generate_table([["text",0],["id",1],["time",2],["time",3],["time",4],
        ["time",5],["int",6],["int", 7],["perc","float",2],["perc","int",6],["code_loc",8]],
        "Rel_table_body",
    "rel");
}

function gen_rul_table() {
    generate_table([["text",0],["id",1],["time",2],["time",3],["time",4],
            ["int",5],["perc","float",2],["perc","int",5],["code_loc",6]],
        "Rul_table_body",
        "rul");
}

function gen_top_rel_table() {
    generate_table([["text",0],["id",1],["time",2],["time",3],["time",4],
        ["time",5],["int",6],["int",7],["perc","float",2],["perc","int",6],["code_loc",8]],
        "top_rel_table_body",
    "topRel");
}

function gen_top_rul_table() {
    generate_table([["text",0],["id",1],["time",2],["time",3],["time",4],
            ["int",5],["perc","float",2],["perc","int",5],["code_loc",6]],
        "top_rul_table_body",
        "topRul");
}


function genRulesOfRelations() {
    var data_format = [["text",0],["id",1],["time",2],["time",3],["time",4],
            ["int",5],["perc","float",2],["perc","int",5],["code_loc",6]];
    var rules = data.rel[selected.rel][9];
    var perc_totals = [];
    var row, cell, perc_counter, table_body, i, j;
    table_body = document.getElementById("rulesofrel_body");
    table_body.innerHTML = "";

    for (i in data_format) {
        if (!data_format.hasOwnProperty(i)) continue;
        if (data_format[i][0] === "perc") {
            perc_totals.push([data_format[i][1],data_format[i][2],0]);
        }
    }

    for (item in rules) {
        if (data.rul[rules[item]].hasOwnProperty(item)) {
            for (i in perc_totals) {
                if (!isNaN(data.rul[rules[item]][perc_totals[i][1]])) {
                    perc_totals[i][2] += data.rul[rules[item]][perc_totals[i][1]];
                }
            }
        }
    }

    for (j=0; j<rules.length; j++) {
        row = document.createElement("tr");
        perc_counter=0;
        for (i in data_format) {
            if (!data_format.hasOwnProperty(i)) continue;
            if (data_format[i][0] === "perc") {
                cell = create_cell(data_format[i][0], data.rul[rules[j]][data_format[i][2]], perc_totals[perc_counter++][2]);
            } else {
                cell = create_cell(data_format[i][0], data.rul[rules[j]][data_format[i][1]]);
            }
            row.appendChild(cell);
        }
        table_body.appendChild(row);
    }

    document.getElementById("rulesofrel").style.display = "block";
}

function genRulVer() {
    var data_format = [["text",0],["id",1],["time",2],["time",3],["time",4],
        ["int",5],["int",7],["perc","float",2],["perc","int",5],["code_loc",6]];
    var rules = data.rul[selected.rul][7];
    var perc_totals = [];
    var row, cell, perc_counter, table_body, i, j;
    table_body = document.getElementById("rulver_body");
    table_body.innerHTML = "";

    for (i in data_format) {
        if (!data_format.hasOwnProperty(i)) continue;
        if (data_format[i][0] === "perc") {
            if (!isNaN(data.rul[selected.rul][data_format[i][2]])) {
                perc_totals.push([data_format[i][1], data_format[i][2], data.rul[selected.rul][data_format[i][2]]]);
            }
        }
    }

    row = document.createElement("tr");
    for (i in data_format) {
        if (!data_format.hasOwnProperty(i)) continue;
        if (data_format[i][1] == 7) {
            cell = create_cell("text","-")
        } else if (data_format[i][0] === "perc") {
            cell = create_cell(data_format[i][0], 1, 1);
        } else {
            cell = create_cell(data_format[i][0], data.rul[selected.rul][data_format[i][1]]);
        }
        row.appendChild(cell);
    }
    table_body.appendChild(row);

    for (j=0; j<rules.length; j++) {
        row = document.createElement("tr");
        perc_counter=0;
        for (i in data_format) {
            if (!data_format.hasOwnProperty(i)) continue;
            if (data_format[i][0] === "perc") {
                cell = create_cell(data_format[i][0], rules[j][data_format[i][2]], perc_totals[perc_counter++][2]);
            } else {
                cell = create_cell(data_format[i][0], rules[j][data_format[i][1]]);
            }
            row.appendChild(cell);
        }
        table_body.appendChild(row);
    }

    document.getElementById("rulver").style.display = "block";
}

function genAtomVer() {
    var atoms = data.atoms[selected.rul];
    var table_body = document.getElementById('atoms_body');
    table_body.innerHTML = "";

    for (i = 0; i < atoms.length; ++i) {
        var row = document.createElement("tr");
        row.appendChild(create_cell("text", atoms[i][1]));
        if (atoms[i][2] === undefined) {
            row.appendChild(create_cell("text", '--'));
        } else {
            row.appendChild(create_cell("int", atoms[i][2]));
        }
        row.appendChild(create_cell("int", atoms[i][3]));
        table_body.appendChild(row);
    }
    document.getElementById("atoms").style.display = "block";
}

function genConfig() {
    var table = document.createElement("table");
    {
        var header = document.createElement("thead");
        var headerRow = document.createElement("tr");
        var headerName = document.createElement("th");
        headerName.textContent = "Key";
        var headerValue = document.createElement("th");
        headerValue.textContent = "Value";

        headerRow.appendChild(headerName);
        headerRow.appendChild(headerValue);
        header.appendChild(headerRow);
        table.appendChild(header);
    }
    var body = document.createElement("tbody");
    for (i in data["configuration"]) {
        var row = document.createElement("tr");
        var name = document.createElement("td");
        if (i === "") {
            name.textContent = "Datalog input file";
        } else {
            name.textContent = i;
        }
        var value = document.createElement("td");
        value.textContent = data["configuration"][i];

        row.appendChild(name);
        row.appendChild(value);
        body.appendChild(row);
    }
    table.appendChild(body);
    return table;
}

function gen_top() {
    var statsElement, line1, line2;
    statsElement = document.getElementById("top-stats");
    line1 = document.createElement("p");
    line1.textContent = "Total runtime: " + humanise_time(data.top[0]) + " (" + data.top[0] + " seconds)";
    line2 = document.createElement("p");
    line2.textContent = "Total tuples: " + minify_numbers(data.top[1]) + " (" + data.top[1] + ")";
    line3 = document.createElement("p");
    line3.textContent = "Total loadtime: " + humanise_time(data.top[2]) + " (" + data.top[2] + " seconds)";
    line4 = document.createElement("p");
    line4.textContent = "Total savetime: " + humanise_time(data.top[3]) + " (" + data.top[3] + " seconds)";
    statsElement.appendChild(line1);
    statsElement.appendChild(line2);
    statsElement.appendChild(line3);
    statsElement.appendChild(line4);
    graphUsages();

    document.getElementById("top-config").appendChild(genConfig());
    gen_top_rel_table();
    gen_top_rul_table();
}

function view_code_snippet(value) {
    value = value.split(" ");
    value = value[value.length-1]; // get [num:num]
    value = value.slice(1); // cut out "["
    value = value.split(":");
    value = value[0];
    var targetLi = gen_code(parseInt(value));
    document.getElementById("code_tab").click();

    var list = document.getElementById("code-view");

    list.scrollTop = Math.max(21*parseInt(value) - 100, 0);

}


function gen_code(highlight_row) {
    var list, row, text, target_row;
    list = document.getElementById("code-list");
    list.innerHTML = "";
    for (var i=0; i<data.code.length; i++) {
        row = document.createElement("li");
        row.className = "code-li";
        if (i+1 == highlight_row) {
            target_row = row;
            row.style.background = "#E0FFFF";
        }
        row.style.marginBottom = "0";
        text = document.createElement("span");
        text.className = "text-span";
        text.textContent = data.code[i];
        row.appendChild(text);
        list.appendChild(row);
    }
    document.getElementById("code-view").appendChild(list);
    return target_row;
}


var precision = !1;
var selected = {rel: !1, rul: !1};
var came_from = !1;
var graph_vals = {
    labels:[],
    tot_t:[],
    tuples:[]
};


function init() {
    gen_top();
    gen_rel_table();
    gen_rul_table();
    gen_code(-1)
    Tablesort(document.getElementById('Rel_table'),{descending: true});
    Tablesort(document.getElementById('Rul_table'),{descending: true});
    Tablesort(document.getElementById('rulesofrel_table'),{descending: true});
    Tablesort(document.getElementById('rulvertable'),{descending: true});
    document.getElementById("default").click();
    //document.getElementById("default").classList['active'] = !0;

}

init();
    )___";
std::string jsTableSort = R"___(
/*!
 * tablesort v4.1.0 (2016-12-29)
 * http://tristen.ca/tablesort/demo/
 * Copyright (c) 2016 ; Licensed MIT
 */
(function() {
    function Tablesort(el, options) {
        if (!(this instanceof Tablesort)) return new Tablesort(el, options);

        if (!el || el.tagName !== 'TABLE') {
            throw new Error('Element must be a table');
        }
        this.init(el, options || {});
    }

    var sortOptions = [];

    var createEvent = function(name) {
        var evt;

        if (!window.CustomEvent || typeof window.CustomEvent !== 'function') {
            evt = document.createEvent('CustomEvent');
            evt.initCustomEvent(name, false, false, undefined);
        } else {
            evt = new CustomEvent(name);
        }

        return evt;
    };

    var getInnerText = function(el) {
        return el.getAttribute('data-sort') || el.textContent || el.innerText || '';
    };

    // Default sort method if no better sort method is found
    var caseInsensitiveSort = function(a, b) {
        a = a.toLowerCase();
        b = b.toLowerCase();

        if (a === b) return 0;
        if (a < b) return 1;

        return -1;
    };

    // Stable sort function
    // If two elements are equal under the original sort function,
    // then there relative order is reversed
    var stabilize = function(sort, antiStabilize) {
        return function(a, b) {
            var unstableResult = sort(a.td, b.td);

            if (unstableResult === 0) {
                if (antiStabilize) return b.index - a.index;
                return a.index - b.index;
            }

            return unstableResult;
        };
    };

    Tablesort.extend = function(name, pattern, sort) {
        if (typeof pattern !== 'function' || typeof sort !== 'function') {
            throw new Error('Pattern and sort must be a function');
        }

        sortOptions.push({
            name: name,
            pattern: pattern,
            sort: sort
        });
    };

    Tablesort.prototype = {

        init: function(el, options) {
            var that = this,
                firstRow,
                defaultSort,
                i,
                cell;

            that.table = el;
            that.thead = false;
            that.options = options;

            if (el.rows && el.rows.length > 0) {
                if (el.tHead && el.tHead.rows.length > 0) {
                    for (i = 0; i < el.tHead.rows.length; i++) {
                        if (el.tHead.rows[i].getAttribute('data-sort-method') === 'thead') {
                            firstRow = el.tHead.rows[i];
                            break;
                        }
                    }
                    if (!firstRow) {
                        firstRow = el.tHead.rows[el.tHead.rows.length - 1];
                    }
                    that.thead = true;
                } else {
                    firstRow = el.rows[0];
                }
            }

            if (!firstRow) return;

            var onClick = function() {
                if (that.current && that.current !== this) {
                    that.current.removeAttribute('aria-sort');
                }

                that.current = this;
                that.sortTable(this);
            };

            // Assume first row is the header and attach a click handler to each.
            for (i = 0; i < firstRow.cells.length; i++) {
                cell = firstRow.cells[i];
                cell.setAttribute('role','columnheader');
                if (cell.getAttribute('data-sort-method') !== 'none') {
                    cell.tabindex = 0;
                    cell.addEventListener('click', onClick, false);

                    if (cell.getAttribute('data-sort-default') !== null) {
                        defaultSort = cell;
                    }
                }
            }

            if (defaultSort) {
                that.current = defaultSort;
                that.sortTable(defaultSort);
            }
        },

        sortTable: function(header, update) {
            var that = this,
                column = header.cellIndex,
                sortFunction = caseInsensitiveSort,
                item = '',
                items = [],
                i = that.thead ? 0 : 1,
                sortMethod = header.getAttribute('data-sort-method'),
                sortOrder = header.getAttribute('aria-sort');

            that.table.dispatchEvent(createEvent('beforeSort'));

            // If updating an existing sort, direction should remain unchanged.
            if (!update) {
                if (sortOrder === 'ascending') {
                    sortOrder = 'descending';
                } else if (sortOrder === 'descending') {
                    sortOrder = 'ascending';
                } else {
                    sortOrder = that.options.descending ? 'ascending' : 'descending';
                }

                header.setAttribute('aria-sort', sortOrder);
            }

            if (that.table.rows.length < 2) return;

            // If we force a sort method, it is not necessary to check rows
            if (!sortMethod) {
                while (items.length < 3 && i < that.table.tBodies[0].rows.length) {
                    item = getInnerText(that.table.tBodies[0].rows[i].cells[column]);
                    item = item.trim();

                    if (item.length > 0) {
                        items.push(item);
                    }

                    i++;
                }

                if (!items) return;
            }

            for (i = 0; i < sortOptions.length; i++) {
                item = sortOptions[i];

                if (sortMethod) {
                    if (item.name === sortMethod) {
                        sortFunction = item.sort;
                        break;
                    }
                } else if (items.every(item.pattern)) {
                    sortFunction = item.sort;
                    break;
                }
            }

            that.col = column;

            for (i = 0; i < that.table.tBodies.length; i++) {
                var newRows = [],
                    noSorts = {},
                    j,
                    totalRows = 0,
                    noSortsSoFar = 0;

                if (that.table.tBodies[i].rows.length < 2) continue;

                for (j = 0; j < that.table.tBodies[i].rows.length; j++) {
                    item = that.table.tBodies[i].rows[j];
                    if (item.getAttribute('data-sort-method') === 'none') {
                        // keep no-sorts in separate list to be able to insert
                        // them back at their original position later
                        noSorts[totalRows] = item;
                    } else {
                        // Save the index for stable sorting
                        newRows.push({
                            tr: item,
                            td: getInnerText(item.cells[that.col]),
                            index: totalRows
                        });
                    }
                    totalRows++;
                }
                // Before we append should we reverse the new array or not?
                // If we reverse, the sort needs to be `anti-stable` so that
                // the double negatives cancel out
                if (sortOrder === 'descending') {
                    newRows.sort(stabilize(sortFunction, true));
                    newRows.reverse();
                } else {
                    newRows.sort(stabilize(sortFunction, false));
                }

                // append rows that already exist rather than creating new ones
                for (j = 0; j < totalRows; j++) {
                    if (noSorts[j]) {
                        // We have a no-sort row for this position, insert it here.
                        item = noSorts[j];
                        noSortsSoFar++;
                    } else {
                        item = newRows[j - noSortsSoFar].tr;
                    }

                    // appendChild(x) moves x if already present somewhere else in the DOM
                    that.table.tBodies[i].appendChild(item);
                }
            }

            that.table.dispatchEvent(createEvent('afterSort'));
        },

        refresh: function() {
            if (this.current !== undefined) {
                this.sortTable(this.current, true);
            }
        }
    };

    if (typeof module !== 'undefined' && module.exports) {
        module.exports = Tablesort;
    } else {
        window.Tablesort = Tablesort;
    }
})();

)___";
std::string jsUtil = R"___(
/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2017, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */


function clean_percentages(data) {
    if (data < 1) {
        return data.toFixed(3);
    }
    return data.toPrecision(3);
}

function humanise_time(time) {
    if (precision) return time.toString();
    if (time < 1e-9) {
        return '0';
    }
    if (time < 1) {
        milli = time * 1000.0;
        if (milli > 1) {
            return time.toFixed(3) + "s"
        }
        micro = milli * 1000.0;
        if (micro >= 1) {
            return micro.toFixed(1) + "µs"
        }
        return (micro*1000).toFixed(1) + "ns"
    } else {
        minutes = (time / 60.0);
        if (minutes < 3) return time.toPrecision(3) + "s";
        hours = (minutes / 60);
        if (hours < 3) return minutes.toFixed(2) + "m";
        days = (hours / 24);
        if (days < 3) return hours.toFixed(2) + "H";
        weeks = (days / 7);
        if (weeks < 3) return days.toFixed(2) + "D";
        year = (days / 365);
        if (year < 3) return weeks.toFixed(2) + "W";
        return year.toFixed(2) + "Y"
    }
}

function minify_memory(value) {
    if (value < 1024 * 10) {
        return value + 'B';
    } else if (value < 1024 * 1024 * 10) {
        return Math.round(value / 1024) + 'kB';
    } else if (value < 1024 * 1024 * 1024 * 10) {
        return Math.round(value / (1024 * 1024)) + 'MB';
    } else if (value < 1024 * 1024 * 1024 * 1024 * 10) {
        return Math.round(value / Math.round(1024 * 1024 * 1024)) + 'GB';
    } else {
        return Math.round(value / Math.round(1024 * 1024 * 1024 * 1024)) + 'TB';
    }
}

function minify_numbers(num) {
    if (precision) return num.toString();
    kilo = (num / 1000);
    if (kilo < 1) return num;
    mil = (kilo / 1000);
    if (mil < 1) return kilo.toPrecision(3) + "K";
    bil = (mil / 1000);
    if (bil < 1) return mil.toPrecision(3) + "M";
    tril = (bil / 1000);
    if (tril < 1) return bil.toPrecision(3) + "B";
    quad = (tril / 1000);
    if (quad < 1) return tril.toPrecision(3) + "T";
    quin = (quad / 1000);
    if (quin < 1) return quad.toPrecision(3) + "q";
    sex = (quin / 1000);
    if (sex < 1) return quin.toPrecision(3) + "Q";
    sept = (sex / 1000);
    if (sept < 1) return sex.toPrecision(3) + "s";
    return sept.toFixed(2) + "S"
}



(function () {
    var cleanNumber = function (x) {
        var num = x.slice(0, -1);
        var spec = x.slice(-1);
        if (spec == 'K') {
            return parseFloat(num) * 1e3;
        } else if (spec == 'M') {
            return parseFloat(num) * 1e6;
        } else if (spec == "B") {
            return parseFloat(num) * 1e9;
        } else if (spec == "T") {
            return parseFloat(num) * 1e12;
        } else if (spec == "q") {
            return parseFloat(num) * 1e15;
        } else if (spec == "Q") {
            return parseFloat(num) * 1e18;
        } else if (spec == "s") {
            return parseFloat(num) * 1e21;
        } else if (spec == "S") {
            return parseFloat(num) * 1e24;
        }
        return parseFloat(x);
    };
    var a = function (a) {
        return a;
    }, b = function (a, b) {
        return a = cleanNumber(a), b = cleanNumber(b), a = isNaN(a) ? 0 : a, b = isNaN(b) ? 0 : b, a - b
    };
    Tablesort.extend("number", function (a) {
        return a.match(/.*/)
    }, function (c, d) {
        return c = a(c), d = a(d), b(d, c)
    })
})();

(function () {
    var compare = function (a, b) {
        return a.localeCompare(b);
    };
    Tablesort.extend("text", function (a) {
        return a.match(/.*/)
    }, function (c, d) {
        return compare(d, c)
    })
})();

(function () {
    var cleanNumber = function (x) {
            if (x.slice(-1) == 'Y') {
                return parseFloat(x.slice(0, -1)) * 365 * 24 * 60 * 60;
            } else if (x.slice(-1) == 'W') {
                return parseFloat(x.slice(0, -1)) * 7 * 24 * 60 * 60;
            } else if (x.slice(-1) == "D") {
                return parseFloat(x.slice(0, -1)) * 24 * 60 * 60;
            } else if (x.slice(-1) == "H") {
                return parseFloat(x.slice(0, -1)) * 60 * 60;
            } else if (x.slice(-1) == "m") {
                return parseFloat(x.slice(0, -1)) * 60;
            } else if (x.slice(-2) == "µs") {
                return parseFloat(x.slice(0, -2)) / 1e6;
            } else if (x.slice(-2) == "ns") {
                return parseFloat(x.slice(0, -2)) / 1e9;
            } else if (x.slice(-1) == "s") {
                return parseFloat(x.slice(0, -1));
            }
            return parseFloat(x);
        },
        compareNumber = function (a, b) {
            a = isNaN(a) ? 0 : a;
            b = isNaN(b) ? 0 : b;
            return a - b;
        };
    Tablesort.extend('time', function (item) {
        return true;
    }, function (a, b) {
        a = cleanNumber(a);
        b = cleanNumber(b);
        return compareNumber(b, a);
    });
}());

)___";
std::string htmlHeadTop = R"___(
<!--
* Souffle - A Datalog Compiler
* Copyright (c) 2017, The Souffle Developers. All rights reserved
* Licensed under the Universal Permissive License v 1.0 as shown at:
* - https://opensource.org/licenses/UPL
* - <souffle root>/licenses/SOUFFLE-UPL.txt
-->
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Souffle Profiler</title>
)___";
std::string htmlHeadBottom = R"___(
</head>
)___";

std::string htmlBodyTop = R"___(
<body>
<div id="wrapper" style="width:100%;height:inherit;">
    <ul class="tab">
        <li><a class="tablinks" id="default" onclick="changeTab(event, 'Top');">Top</a></li>
        <li><a class="tablinks" id="rel_tab" onclick="changeTab(event, 'Relations');came_from = 'rel';">Relations</a></li>
        <li><a class="tablinks" id="rul_tab" onclick="changeTab(event, 'Rules');came_from = 'rul';">Rules</a></li>
        <li id="code-tab"><a class="tablinks" id="code_tab" onclick="changeTab(event, 'Code')">Code</a></li>
        <li><a class="tablinks" onclick="changeTab(event, 'Help')">Help</a></li>
        <li id="chart-tab" style="display:none;"><a id="chart_tab" onclick="changeTab(event, 'Chart')" class="tablinks">Chart</a></li>
    </ul>
    <div id="Help" class="tabcontent" style="max-width:800px;margin-left: auto;margin-right: auto;">
        <h3>GUI Souffle profiler</h3>
        <p>Select Relation or Rules from the top bar to see a table of Relations or Rules</p>
        <p>Tables show:</p>
        <ul>
            <li>Name/description</li>
            <li>ID(generated by profiler)</li>
            <li>Total time the rule/relation was being processed</li>
            <li>Non-recursive time</li>
            <li>Recursive time</li>
            <li>Copy time</li>
            <li>number of Tuples of the Rule/relation</li>
            <li>Percentage of time the rule/relation ran in comparison to the total</li>
            <li>Percentage of tuples generated by the rule/relation compared to the total</li>
            <li>The source location of the rule/relation in the datalog file</li>
        </ul>
        <p>The tables are sortable by all columns by clicking on the header. Number precision can be toggled by pressing the button at the top of the page to show either shorthand or full precision.</p>
        <p>In the relation tab, to see the rules of a relation, select a relation from the table, and a table of rules will appear below. Similary, by selecting a Rule in the Rule tab, a list of versions of the rule will show up (for recursive rules).</p>
        <p>To visualise a graph of a relation, select the relation from the Relations table, then press the graph selected button to show the iterations of the Relation</p>
        <p>Similarly for a Rule, in the Rules table, select a rule, and select either graph the selected rule's iterations or the versions of the selected rule.</p>
    </div>
    <div id="Top" class="tabcontent" style="margin-left: auto;margin-right: auto;">
        <h3>Top</h3>
        <div id="top-stats"></div>
        <h3>Slowest relations to compute</h1>
        <div class="table_wrapper">
            <table id='top_rel_table'>
                <thead>
                <tr>
                    <th data-sort-method="text">Name</th>
                    <th data-sort-method="text">ID</th>
                    <th data-sort-method="time">Total Time</th>
                    <th data-sort-method="time">Non Rec Time</th>
                    <th data-sort-method="time">Rec Time</th>
                    <th data-sort-method="time">Copy Time</th>
                    <th data-sort-method="number">Tuples</th>
                    <th data-sort-method="number">Reads</th>
                    <th data-sort-method="number">% of Time</th>
                    <th data-sort-method="number">% of Tuples</th>
                    <th data-sort-method="text">Source</th>
                </tr>
                </thead>
                <tbody id="top_rel_table_body">
                </tbody>
            </table>
        </div>
        <h3>Slowest rules to compute</h1>
        <div class="table_wrapper">
            <table id='top_rul_table'>
                <thead>
                <tr>
                    <th data-sort-method="text">Name</th>
                    <th data-sort-method="text">ID</th>
                    <th data-sort-method="time">Total Time</th>
                    <th data-sort-method="time">Non Rec Time</th>
                    <th data-sort-method="time">Rec Time</th>
                    <th data-sort-method="number">Tuples</th>
                    <th data-sort-method="number">% of Time</th>
                    <th data-sort-method="number">% of Tuples</th>
                    <th data-sort-method="text">Source</th>
                </tr>
                </thead>
                <tbody id="top_rul_table_body">
                </tbody>
            </table>
        </div>
        <div id="top-graphs">
            <h3>CPU time</h1>
            <div class="ct-chart-cpu"></div>
            <h3>Maximum Resident Set Size</h1>
            <div class="ct-chart-rss"></div>
        </div>
        <div id="top-config"></div>
    </div>
    <div id="Relations" class="tabcontent">
        <h3>Relations table</h3>
        <button onclick="toggle_precision();">Toggle number precision</button>
        <button onclick="graphRel();">Graph iterations of selected</button>
    <div class="table_wrapper">
        <table id='Rel_table'>
            <thead>
            <tr>
                <th data-sort-method="text">Name</th>
                <th data-sort-method="text">ID</th>
                <th data-sort-method="time">Total Time</th>
                <th data-sort-method="time">Non Rec Time</th>
                <th data-sort-method="time">Rec Time</th>
                <th data-sort-method="time">Copy Time</th>
                <th data-sort-method="number">Tuples</th>
                <th data-sort-method="number">Reads</th>
                <th data-sort-method="number">% of Time</th>
                <th data-sort-method="number">% of Tuples</th>
                <th data-sort-method="text">Source</th>
            </tr>
            </thead>
            <tbody id="Rel_table_body">
            </tbody>
        </table>
    </div>
    <hr/>
    <div id="rulesofrel" style="display:none;">
        <h3>Rules of Relation</h3>
        <div class="table_wrapper">
            <table id="rulesofrel_table">
                <thead>
                <tr>
                    <th data-sort-method="text" style="width:80%;">Name</th>
                    <th data-sort-method="text">ID</th>
                    <th data-sort-method="time">Total Time</th>
                    <th data-sort-method="time">Non Rec Time</th>
                    <th data-sort-method="time">Rec Time</th>
                    <th data-sort-method="number">Tuples</th>
                    <th data-sort-method="number">% of Time</th>
                    <th data-sort-method="number">% of Tuples</th>
                    <th data-sort-method="text" style="width:20%;">Source</th>
                </tr>
                </thead>
                <tbody id="rulesofrel_body">

                </tbody>
            </table>
        </div>
    </div>
</div>
<div id="Rules" class="tabcontent">
    <h3>Rules table</h3>
    <button onclick="toggle_precision();">Toggle number precision</button>
    <button onclick="graphIterRul();">Graph iterations of selected</button>
    <div class="table_wrapper">
        <table id='Rul_table'>
            <thead>
            <tr>
                <th data-sort-method="text">Name</th>
                <th data-sort-method="text">ID</th>
                <th data-sort-method="time">Total Time</th>
                <th data-sort-method="time">Non Rec Time</th>
                <th data-sort-method="time">Rec Time</th>
                <th data-sort-method="number">Tuples</th>
                <th data-sort-method="number">% of Time</th>
                <th data-sort-method="number">% of Tuples</th>
                <th data-sort-method="text">Source</th>
            </tr>
            </thead>
            <tbody id="Rul_table_body">
            </tbody>
        </table>
    </div>
    <hr/>
    <div id="rulver" style="display:none;">
        <h3>Rule Versions Table</h3>
        <div class="table_wrapper">
            <table id='rulvertable'>
                <thead>
                <tr>
                    <th data-sort-method="text">Name</th>
                    <th data-sort-method="text">ID</th>
                    <th data-sort-method="time">Total Time</th>
                    <th data-sort-method="time">Non Rec Time</th>
                    <th data-sort-method="time">Rec Time</th>
                    <th data-sort-method="number">Tuples</th>
                    <th data-sort-method="number">Ver</th>
                    <th data-sort-method="number">% of Time</th>
                    <th data-sort-method="number">% of Tuples</th>
                    <th data-sort-method="text">Source</th>
                </tr>
                </thead>
                <tbody id="rulver_body">
                </tbody>
            </table>
        </div>
    </div>
    <div id="atoms" style="display:none;">
        <h3>Atom Frequency Table</h3>
        <div class="table_wrapper">
            <table id='atomstable'>
                <thead>
                <tr>
                    <th data-sort-method="text">Name</th>
                    <th data-sort-method="text">Relation Size</th>
                    <th data-sort-method="text">Frequency</th>
                </tr>
                </thead>
                <tbody id="atoms_body">
                </tbody>
            </table>
        </div>
    </div>
</div>
<div id="Chart" class="tabcontent">
    <button onclick="goBack(event)">Go Back</button>
    <button onclick="toggle_precision();">Toggle number precision</button>
    <h1>Total run time</h1>
    <div class="ct-chart1"></div>
    <h1>Total number of tuples</h1>
    <div class="ct-chart2"></div>
    <!--<h1>Copy time</h1>-->
    <!--<div class="ct-chart3"></div>-->
    <!--<button onclick="show_graph_vals=!show_graph_vals;draw_graph();">Toggle values</button>-->
</div>
<div id="Code" class="tabcontent">
    <h3>Source Code</h3>
    <div id="code-view">
        <ol id="code-list"></ol>
    </div>
</div>
</div>
)___";
std::string htmlBodyBottom = R"___(
</body>
</html>

)___";
}  // namespace

std::string HtmlGenerator::getHtml(std::string json) {
    return getFirstHalf() + json + getSecondHalf();
}

std::string HtmlGenerator::getFirstHalf() {
    std::stringstream ss;
    ss << htmlHeadTop << HtmlGenerator::wrapCss(cssChartist) << wrapCss(cssStyle) << htmlHeadBottom
       << htmlBodyTop << "<script>data=";
    return ss.str();
}
std::string HtmlGenerator::getSecondHalf() {
    std::stringstream ss;
    ss << "</script>" << wrapJs(jsTableSort) << wrapJs(jsChartistMin) << wrapJs(jsChartistPlugin)
       << wrapJs(jsUtil) << wrapJs(jsMain) << htmlBodyBottom;
    return ss.str();
}
std::string HtmlGenerator::wrapCss(const std::string& css) {
    return "<style>" + css + "</style>";
}
std::string HtmlGenerator::wrapJs(const std::string& js) {
    return "<script>" + js + "</script>";
}

}  // namespace profile
}  // namespace souffle
