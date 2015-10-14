-- initialize the server
ua = require("open62541")
server = ua.server(16664)

-- entity objecttype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.baseobjecttype
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.qualifiedname(1, "Entity")
local attr = ua.types.objecttypeattributes()
attr.displayname = ua.types.localizedtext("en_US", "Entity")
local entity_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                           reference_nodeid, browsename, attr)

-- interface objecttype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.baseobjecttype
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.qualifiedname(1, "Interface")
local attr = ua.types.objecttypeattributes()
attr.displayname = ua.types.localizedtext("en_US", "Interface")
local interface_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                              reference_nodeid, browsename, attr)

-- connectedwith referencetype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.nonhierarchicalreferences
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.qualifiedname(1, "ConnectedWith")
local attr = ua.types.referencetypeattributes()
attr.displayname = ua.types.localizedtext("en_US", "ConnectedWith")
attr.symmetric = true
connectedwith_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid,
                                               reference_nodeid, browsename, attr)

-- administrationshell objecttype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.baseobjecttype
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.qualifiedname(1, "AdministrationShell")
local attr = ua.types.objecttypeattributes()
attr.displayname = ua.types.localizedtext("en_US", "AdministrationShell")
local as_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                           reference_nodeid, browsename, attr)

-- administrates referencetype
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.nonhierarchicalreferences
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.qualifiedname(1, "Administrates")
local attr = ua.types.referencetypeattributes()
attr.displayname = ua.types.localizedtext("en_US", "Administrates")
attr.inversename = ua.types.localizedtext("en_US", "AdministeredBy")
local administrates_id = server:addReferenceTypeNode(requested_nodeid, parent_nodeid,
                                                     reference_nodeid, browsename, attr)

-- usb interface
local requested_nodeid = ua.nodeids.null
local parent_nodeid = interface_id
local reference_nodeid = ua.nodeids.hassubtype
local browsename = ua.types.qualifiedname(1, "USBInterface")
local attr = ua.types.objecttypeattributes()
attr.displayname = ua.types.localizedtext("en_US", "USBInterface")
local usb_id = server:addObjectTypeNode(requested_nodeid, parent_nodeid,
                                              reference_nodeid, browsename, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.qualifiedname(1, "USBVersion")
local typeidentifier = ua.nodeids.null
local attr = ua.types.variableattributes()
attr.displayname = ua.types.localizedtext("en_US", "USBVersion")
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid,
                            browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = usb_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.qualifiedname(1, "USBConnectorType")
local typeidentifier = ua.nodeids.null
local attr = ua.types.variableattributes()
attr.displayname = ua.types.localizedtext("en_US", "USBConnectorType")
id = server:addVariableNode(requested_nodeid, parent_nodeid, reference_nodeid,
                            browsename, typeidentifier, attr)

-- entities
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.objects
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "Entities")
local typeidentifier = ua.nodeids.foldertype
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "Entities")
entities_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- office
local requested_nodeid = ua.nodeids.null
local parent_nodeid = entities_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "Office")
local typeidentifier = entity_id
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "Office")
office_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer
local requested_nodeid = ua.nodeids.null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "Computer")
local typeidentifier = entity_id
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "Computer")
computer_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                   reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.qualifiedname(1, "Speed")
local typeidentifier = ua.nodeids.null
local attr = ua.types.variableattributes()
attr.displayname = ua.types.localizedtext("en_US", "Speed")
server:addVariableNode(requested_nodeid, parent_nodeid,
                       reference_nodeid, browsename, typeidentifier, attr)

-- computer usb
local requested_nodeid = ua.nodeids.null
local parent_nodeid = computer_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "ComputerUSB")
local typeidentifier = usb_id
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "ComputerUSB")
local computerusb_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                            reference_nodeid, browsename, typeidentifier, attr)

-- mouse
local requested_nodeid = ua.nodeids.null
local parent_nodeid = office_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "Mouse")
local typeidentifier = entity_id
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "Mouse")
mouse_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                reference_nodeid, browsename, typeidentifier, attr)

local requested_nodeid = ua.nodeids.null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeids.hasproperty
local browsename = ua.types.qualifiedname(1, "Resolution")
local typeidentifier = ua.nodeids.null
local attr = ua.types.variableattributes()
attr.displayname = ua.types.localizedtext("en_US", "Resolution")
server:addVariableNode(requested_nodeid, parent_nodeid,
                       reference_nodeid, browsename, typeidentifier, attr)

-- mouse usb
local requested_nodeid = ua.nodeids.null
local parent_nodeid = mouse_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "MouseUSB")
local typeidentifier = usb_id
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "MouseUSB")
local mouseusb_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                         reference_nodeid, browsename, typeidentifier, attr)

-- mouse <--> computer usb
local source = computerusb_id
local reftype = connectedwith_id
local target = mouseusb_id
local isforward = false
server:addReference(source, reftype, target, isforward)

-- shells
local requested_nodeid = ua.nodeids.null
local parent_nodeid = ua.nodeids.objects
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "Administration Shells")
local typeidentifier = ua.nodeids.foldertype
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "Administration Shells")
shells_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell
local requested_nodeid = ua.nodeids.null
local parent_nodeid = shells_id
local reference_nodeid = ua.nodeids.organizes
local browsename = ua.types.qualifiedname(1, "Computer Admin Shell")
local typeidentifier = as_id
local attr = ua.types.objectattributes()
attr.displayname = ua.types.localizedtext("en_US", "Computer Admin Shell")
cas_id = server:addObjectNode(requested_nodeid, parent_nodeid,
                                 reference_nodeid, browsename, typeidentifier, attr)

-- computer admin shell -> computer
local source = computer_id
local reftype = administrates_id
local target = cas_id
local isforward = false
server:addReference(source, reftype, target, isforward)

-- start the server
server:start()

while true do
   input = io.read("*l")
   if input == "q" then return end
end
