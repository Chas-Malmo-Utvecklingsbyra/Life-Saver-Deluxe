/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Life Saver Deluxe", "index.html", [
    [ "doxygen-comment", "md_docs_2doxygen-comment.html", null ],
    [ "System Overview", "md_docs_2_h_w__documentation.html", [
      [ "Purpose", "md_docs_2_h_w__documentation.html#autotoc_md1", null ],
      [ "Architecture", "md_docs_2_h_w__documentation.html#autotoc_md2", null ],
      [ "Hardware Components", "md_docs_2_h_w__documentation.html#autotoc_md3", [
        [ "1. ESP32-S3", "md_docs_2_h_w__documentation.html#autotoc_md4", [
          [ "Technical Data", "md_docs_2_h_w__documentation.html#autotoc_md5", null ],
          [ "Display", "md_docs_2_h_w__documentation.html#autotoc_md6", null ]
        ] ],
        [ "2. ESP32-C3 Sensor Nodes", "md_docs_2_h_w__documentation.html#autotoc_md7", [
          [ "Connections", "md_docs_2_h_w__documentation.html#autotoc_md8", null ]
        ] ],
        [ "3. Magnetic Sensors", "md_docs_2_h_w__documentation.html#autotoc_md9", [
          [ "Signals", "md_docs_2_h_w__documentation.html#autotoc_md10", null ],
          [ "Example", "md_docs_2_h_w__documentation.html#autotoc_md11", null ]
        ] ],
        [ "4. ESP32-CAM", "md_docs_2_h_w__documentation.html#autotoc_md12", [
          [ "Communication", "md_docs_2_h_w__documentation.html#autotoc_md13", null ]
        ] ],
        [ "5. BME280 Sensor", "md_docs_2_h_w__documentation.html#autotoc_md14", [
          [ "Communication", "md_docs_2_h_w__documentation.html#autotoc_md15", null ],
          [ "Address", "md_docs_2_h_w__documentation.html#autotoc_md16", null ],
          [ "Connections", "md_docs_2_h_w__documentation.html#autotoc_md17", null ]
        ] ]
      ] ],
      [ "Communication Architecture", "md_docs_2_h_w__documentation.html#autotoc_md18", [
        [ "Wi-Fi Network", "md_docs_2_h_w__documentation.html#autotoc_md19", [
          [ "Protocols", "md_docs_2_h_w__documentation.html#autotoc_md20", null ]
        ] ],
        [ "Signal Flow", "md_docs_2_h_w__documentation.html#autotoc_md21", [
          [ "Door/Window Sensor Event", "md_docs_2_h_w__documentation.html#autotoc_md22", null ],
          [ "Environmental Sensor Event (BME280)", "md_docs_2_h_w__documentation.html#autotoc_md23", null ],
          [ "Camera Monitoring", "md_docs_2_h_w__documentation.html#autotoc_md24", null ]
        ] ]
      ] ],
      [ "Power Supply", "md_docs_2_h_w__documentation.html#autotoc_md25", null ],
      [ "Pin Configuration", "md_docs_2_h_w__documentation.html#autotoc_md26", [
        [ "Application GPIO Usage", "md_docs_2_h_w__documentation.html#autotoc_md27", null ],
        [ "Display Interface GPIOs", "md_docs_2_h_w__documentation.html#autotoc_md28", null ],
        [ "Additional Reserved Display GPIOs", "md_docs_2_h_w__documentation.html#autotoc_md29", null ]
      ] ],
      [ "System Limitations", "md_docs_2_h_w__documentation.html#autotoc_md30", null ]
    ] ],
    [ "uml", "md_docs_2uml.html", null ],
    [ "Life-Saver-Deluxe", "md__r_e_a_d_m_e.html", [
      [ "Demo", "md__r_e_a_d_m_e.html#autotoc_md33", [
        [ "Home-Hub", "md__r_e_a_d_m_e.html#autotoc_md34", null ],
        [ "Cam-module:", "md__r_e_a_d_m_e.html#autotoc_md35", null ]
      ] ],
      [ "Overview", "md__r_e_a_d_m_e.html#autotoc_md37", null ],
      [ "Features", "md__r_e_a_d_m_e.html#autotoc_md39", [
        [ "Home Hub (ESP32-S3)", "md__r_e_a_d_m_e.html#autotoc_md40", null ],
        [ "Sensor Nodes (ESP32-C3)", "md__r_e_a_d_m_e.html#autotoc_md41", null ],
        [ "Environmental Monitoring", "md__r_e_a_d_m_e.html#autotoc_md42", null ],
        [ "Camera Module (Experimental)", "md__r_e_a_d_m_e.html#autotoc_md43", null ]
      ] ],
      [ "System Architecture", "md__r_e_a_d_m_e.html#autotoc_md45", null ],
      [ "Technology Stack", "md__r_e_a_d_m_e.html#autotoc_md47", null ],
      [ "Repository Structure", "md__r_e_a_d_m_e.html#autotoc_md49", null ],
      [ "Hardware Requirements", "md__r_e_a_d_m_e.html#autotoc_md51", [
        [ "Home Hub", "md__r_e_a_d_m_e.html#autotoc_md52", null ],
        [ "Sensor Node", "md__r_e_a_d_m_e.html#autotoc_md53", null ],
        [ "Optional", "md__r_e_a_d_m_e.html#autotoc_md54", null ]
      ] ],
      [ "Building the Project", "md__r_e_a_d_m_e.html#autotoc_md56", [
        [ "Prerequisites", "md__r_e_a_d_m_e.html#autotoc_md57", null ],
        [ "Clone Repository", "md__r_e_a_d_m_e.html#autotoc_md58", null ],
        [ "Build", "md__r_e_a_d_m_e.html#autotoc_md59", null ],
        [ "Flash", "md__r_e_a_d_m_e.html#autotoc_md60", null ],
        [ "Monitor", "md__r_e_a_d_m_e.html#autotoc_md61", null ]
      ] ],
      [ "Documentation", "md__r_e_a_d_m_e.html#autotoc_md63", null ],
      [ "Sequence diagram", "md__r_e_a_d_m_e.html#autotoc_md65", null ],
      [ "Development Process", "md__r_e_a_d_m_e.html#autotoc_md67", null ],
      [ "Key Learnings", "md__r_e_a_d_m_e.html#autotoc_md69", null ],
      [ "Team Members", "md__r_e_a_d_m_e.html#autotoc_md71", null ]
    ] ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "Globals", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"allocator_2mocks_2mock__deps_8h.html",
"dir_b21cc758e9e74f54942422dc62014ccb.html",
"gui__sensors_8c.html#acd80ba98a259303b153aeff66c29361a",
"md_docs_2_h_w__documentation.html#autotoc_md4",
"structtheme__t.html#a8f8f80d37794cde9472343e4487ba3eb",
"unity_8h.html#a1c4f73570e4cad0f343902c8426963ec",
"unity_8h.html#aa73f8c4f920f89e565fa19ee10b93356",
"unity__internals_8h.html#a44a50f4cb297bc3524d05b80000aa3a2",
"unity__internals_8h.html#af209be736676b8891766939bb78e8f3a"
];

const SYNCONMSG = 'click to disable panel synchronization';
const SYNCOFFMSG = 'click to enable panel synchronization';
const LISTOFALLMEMBERS = 'List of all members';