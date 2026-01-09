# XRCT Radiation Shielding Solver


When the user clicks New Project:

Step 1 — Project shell is created

Internally:

Empty project object

No background

No scale

Empty geometry

UI state:

Editor still hidden

Only onboarding UI visible

Step 2 — Upload floorplan (PDF)

User clicks Upload Floorplan.

What happens:

File picker opens

User selects a PDF

You convert page 1 → PNG

Store the PNG inside the project

Show preview

At this point:

Grid still disabled

Drawing disabled

Toolbar disabled

Step 3 — Set scale (mandatory)

Now the app forces the user into scale calibration.

UI:

“Click two points that are X cm apart”

Numeric input

Confirm

Once confirmed:

Scale becomes valid

Grid snaps activate

Drawing unlocks

Now the project transitions into Editing Mode.


----------------------------------------------
What to implement next (clear answer)

Your next concrete steps should be:

Add AppMode enum

Create ProjectPicker screen

Add PDF upload → PNG conversion

Add scale calibration UI

Only then:

Save / Load

Anything else before this is premature.