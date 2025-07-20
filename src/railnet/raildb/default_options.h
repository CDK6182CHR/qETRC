#include "data/diagram/diagramoptions.h"

/**
 * 2025.07.20: Provided for RailNet, the default DiagramOptions.
 * Because the railnet part sometimes also calls the widgets used for the main program (e.g., ForbidWidget),
 * the DiagramOptions must be filled then.
 */
extern const DiagramOptions defaultDiagramOptionsForDB;
