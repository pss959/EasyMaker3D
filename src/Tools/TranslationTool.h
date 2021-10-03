#pragma once

#include "Tools/Tool.h"

/// TranslationTool allows interactive translation along any of the principal
/// coordinate axes.
///
/// \ingroup Tools
class TranslationTool : Tool {
    // ------------------------------------------------------------------------
    // Required TransformTool functions.
    // ------------------------------------------------------------------------

    public override GripGuide.GuideType GetGripGuideType() {
        return GripGuide.GuideType.Basic;
    }

    protected override GameObject CreateGeometry() {
        GameObject prefab = UT.InstantiatePrefab("TranslationTool");
        prefab.transform.SetParent(transform, false);

        // Access the Face and Stick template objects.
        GameObject  faceTemplate = UT.FindGameObjectUnder(prefab, "Face");
        GameObject stickTemplate = UT.FindGameObjectUnder(prefab, "Stick");

        // Determine the base scale factor to make each _Face the correct size.
        Bounds faceBounds = UT.GetObjectBounds(faceTemplate);
        _faceScale = FaceSize / faceBounds.size.x;

        // Create 6 Faces.
        _faces = new _Face[6];
        for (int f = 0; f < 6; ++f)
            _faces[f] = CreateFace(faceTemplate, f);

        // Create 3 sticks to join the faces.
        _sticks = new GameObject[3];
        for (int dim = 0; dim < 3; ++dim)
            _sticks[dim] = CreateStick(stickTemplate, dim,
                                       ColorDict.GetDimensionColor(dim));

        // Disable the templates.
        faceTemplate.SetActive(false);
        stickTemplate.SetActive(false);

        return prefab;
    }

    protected override void UpdateGeometry(Model model) {
        _modelSize = MatchModel(model, true);

        // Update the scales and positions of the faces based on the Model
        // size.  Use the computed local bounds and the local scale
        // factors. (The position and rotation are handled above.)
        foreach (_Face face in _faces)
            SetFaceTransform(_modelSize, face);

        // Update the stick lengths.
        for (int dim = 0; dim < 3; ++dim) {
            Transform xf = _sticks[dim].transform;
            xf.localScale =
                UT.ReplaceElement(xf.localScale, 0, _modelSize[dim]);
        }
    }

    // ------------------------------------------------------------------------
    // IGrippable interface.
    // ------------------------------------------------------------------------

    public override void UpdateGripHoverData(GripData data) {
        Vector3 dir = data.GetLocalDir(transform);

        for (int dim = 0; dim < 3; ++dim) {
            bool isOpposite;
            if (AreVectorsClose(dir, UT.GetAxis(dim), out isOpposite)) {
                int faceIndex = 2 * dim + (isOpposite ? 1 : 0);
                _Face face = _faces[faceIndex];
                data.targetPoint = face.faceGO.transform.position;
                data.color       = GetFeedbackColor(dim, false);
                data.go          = face.slider.gameObject;
                return;
            }
        }
        // Nothing was close.
        data.go = null;
    }

    // ------------------------------------------------------------------------
    // Private fields.
    // ------------------------------------------------------------------------

    //! This struct holds information for one of the 6 indexed faces. A face
    // consists of the following subgraph:
    //     GameObject   "Face_N" : rotates, scales, and positions the face.
    //       GameObject "Face"   : from Face prefab; has the SliderWidget.
    private struct _Face {
        public int          index;     //! Index in _faces array.
        public GameObject   faceGO;    //! Parent in subgraph; named "Face_N".
        public GameObject   sliderGO;  //! Child in subgraph.
        public SliderWidget slider;    //! SliderWidget attached to sliderGO.

        //! Returns the dimension the face is perpendicular to.
        public int GetDimension() { return index / 2; }

        //! Returns true if the face is on the maximum side of the Model.
        public bool IsMax() { return (index & 1) != 0; }
    }

    // Some constants to clarify the code.
    private const float FaceSize  =  3f;
    private const float MaxRange  = 40f;  //! Stage coordinates.

    // Various parts.
    private _Face[]         _faces;      //! 6 generated _Face structs.
    private GameObject[]    _sticks;     //! 3 sticks to join faces.

    //! Scale applied to _Face object to get it to proper size for the tool.
    private float          _faceScale;

    //! Command used to modify all affected Models.
    private TranslateCommand _command;

    //! Feedback display.
    private LinearFeedback _feedback;

    // Values set during a drag.
    private Vector3    _modelSize;             //! Size of Model (stage coords).
    private float      _startValue;            //! Starting value of slider.
    private int        _activeFaceIndex = -1;  //! Index of dragged face.

    //! These store the three points in stage coordinates used for potential
    // snapping operations for motion in a given dimension. These are the
    // intersection with the motion vector in that dimension with the min,
    // center, and max of bounds at the start of a drag operation.
    private Vector3 _startStageMin;
    private Vector3 _startStagePos;
    private Vector3 _startStageMax;

    // ------------------------------------------------------------------------
    // Geometry creation and update.
    // ------------------------------------------------------------------------

    //! Creates and returns one face. A face consists of a GameObject with a
    // Transform that rotates, scales, and positions the face. The GameObject's
    // only child is the Face template that has the SliderWidget.
    private _Face CreateFace(GameObject faceTemplate, int index) {
        _Face face = new _Face();
        face.index = index;

        // Create the parent GameObject to hold the transform, which is
        // modified when UpdateGeometry() is called.
        face.faceGO = new GameObject();
        face.faceGO.name = "Face_" + index;
        face.faceGO.transform.SetParent(transform, false);

        // Instantiate the template and add it as a child.
        face.sliderGO = Object.Instantiate(faceTemplate);
        face.sliderGO.name = face.faceGO.name + "_Slider";
        face.sliderGO.transform.SetParent(face.faceGO.transform, false);

        // Attach callbacks to the SliderWidget.
        face.slider = face.sliderGO.GetComponent<SliderWidget>();
        Assert.IsNotNull(face.slider);
        face.slider.isPrecisionBased = true;
        face.slider.Activation.AddListener(FaceActivated);
        face.slider.ValueChanged.AddListener(FaceMoved);

        // Compute the face normal. The default face is oriented with the +X
        // axis. The first 2 faces are perpendicular to the X axis, the next 2
        // Y, the last 2 Z.
        Vector3 normal = Vector3.zero;
        normal[face.GetDimension()] = face.IsMax() ? 1f : -1f;

        // Rotate the face based on the face normal.
        face.faceGO.transform.localRotation =
            Quaternion.FromToRotation(Vector3.right, normal);

        return face;
    }

    //! Creates and returns a stick for the given dimension.
    private GameObject CreateStick(GameObject stickTemplate, int dim,
                                   Color color) {
        GameObject stickGO = Object.Instantiate(stickTemplate);
        stickGO.name = "Stick_" + dim;
        stickGO.transform.SetParent(transform, false);
        stickGO.transform.localRotation =
            Quaternion.FromToRotation(Vector3.right, UT.GetAxis(dim));
        UT.ApplyColorUnder(stickGO, "Dimension", color);
        return stickGO;
    }

    //! Sets the local scale and position in the given Transform for the indexed
    // face based on the given Model bounds. Also sets up the slider min/max
    // values for the appropriate range.
    private void SetFaceTransform(Vector3 size, _Face face) {
        Transform xf = face.faceGO.transform;

        // Scale the face to get it to the proper size.
        xf.localScale = _faceScale * Vector3.one;

        // Determine the dimension based on the index.
        int dim = face.GetDimension();

        // The first 2 faces are perpendicular to the X axis, the next 2 Y, the
        // last 2 Z.
        float hsz = .5f * size[dim];
        xf.localPosition = (face.IsMax() ? hsz : -hsz) * UT.GetAxis(dim);

        // Set up the face slider's range. The center (0) is the current
        // position.
        Assert.IsNotNull(face.slider);
        // TODO: Use location and BuildVolume size to limit range.
        face.slider.minValue = -MaxRange / xf.localScale[dim];
        face.slider.maxValue =  MaxRange / xf.localScale[dim];
        face.slider.currentValue = 0f;
    }

    //! SliderWidget activation callback.
    private void FaceActivated(Widget sender, bool isActivation) {
        Model model = GetModel();
        if (model == null)
            return;

        if (isActivation) {
            // Get the index from the name of the parent of the widget's object
            // ("Face_N").
            int index = int.Parse(sender.transform.parent.name.Substring(5));
            ActivateFace(model, index);
            GetContext().targetManager.StartSnapping();

            // Save the starting points of the translation in stage coordinates.
            int dim = _faces[index].GetDimension();
            Matrix4x4 lsm = model.GetLocalToStageMatrix();
            Vector3 pos  = model.transform.localPosition;
            Vector3 bVec = (.5f * _modelSize[dim]) * UT.GetAxis(dim);
            _startStageMin = lsm.MultiplyPoint3x4(pos - bVec);
            _startStagePos = lsm.MultiplyPoint3x4(pos);
            _startStageMax = lsm.MultiplyPoint3x4(pos + bVec);
        }
        else {
            DeactivateActiveFace(model);
            GetContext().targetManager.EndSnapping();
        }
    }

    private void ActivateFace(Model model, int index) {
        Assert.IsTrue(index >= 0 && index <= 5);
        Assert.IsTrue(_activeFaceIndex < 0);
        _activeFaceIndex = index;

        _Face activeFace = _faces[_activeFaceIndex];
        SliderWidget sw = activeFace.slider;

        // Save the starting information.
        _startValue = sw.currentValue;

        // Turn off all of the other faces and sticks.
        foreach (_Face face in _faces)
            if (face.index != _activeFaceIndex)
                face.faceGO.SetActive(false);
        foreach (GameObject stick in _sticks)
            stick.SetActive(false);

        // Activate the feedback.
        _feedback = ActivateFeedback<LinearFeedback>();
    }

    private void DeactivateActiveFace(Model model) {
        // Set the _activeFaceIndex to -1 so that nothing happens when the
        // following code invokes the callbacks.
        Assert.IsTrue(_activeFaceIndex >= 0);
        _activeFaceIndex = -1;

        // This could be the end of a drag. If there was any motion, execute
        // the command to change the transforms.
        if (_command != null) {
            if (_command.translation != Vector3.zero)
                GetContext().commandManager.AddAndDo(_command);
            _command = null;
        }

        // Turn all the faces and sticks back on and put all the geometry in
        // the right places.
        foreach (_Face face in _faces)
            face.faceGO.SetActive(true);
        foreach (GameObject stick in _sticks)
            stick.SetActive(true);
        UpdateGeometry(model);

        // Deactivate the feedback.
        DeactivateFeedback<LinearFeedback>(_feedback);
        _feedback = null;

        // Invoke the DragEnded callbacks.
        DragEnded.Invoke(this);
    }

    //! SliderWidget motion callback.
    private void FaceMoved(Widget sender, float value) {
        if (_activeFaceIndex < 0)
            return;

        // If this is the first change, create the ScaleCommand and start the
        // drag.
        List<Model> models = GetSelection().GetAll();
        if (_command == null) {
            _command = new TranslateCommand(models);
            DragStarted.Invoke(this);
        }

        // Determine the motion of the slider in its native coordinates
        // (aligned with X axis).
        SliderWidget sw = _faces[_activeFaceIndex].slider;
        float xMotion = sw.currentValue - _startValue;

        // Transform the motion vector into stage coordinates.
        Matrix4x4 lsm = UT.GetLocalToStageMatrix(sw.gameObject);
        Vector3 motion = lsm.MultiplyVector(UT.Vec3(xMotion, 0f, 0f));

        // Try snapping the bounds min, center, and max in the direction of
        // motion to the point target. If nothing snaps, adjust by the current
        // precision.
        bool isSnapped = false;
        if (motion.magnitude > 0f) {
            TargetManager targetMgr = GetContext().targetManager;
            if (targetMgr.SnapToPoint(_startStagePos, ref motion) ||
                targetMgr.SnapToPoint(_startStageMin, ref motion) ||
                targetMgr.SnapToPoint(_startStageMax, ref motion))
                isSnapped = true;
            else
                motion *= Precision.Apply(motion.magnitude) / motion.magnitude;
        }

        // Simulate execution of the command to update all the Models.
        _command.translation = motion;
        GetContext().commandManager.SimulateDo(_command);

        // Update the feedback using the motion vector.
        if (_feedback != null)
            UpdateFeedback(models[0], motion, isSnapped);
    }

    //! Updates the feedback during a drag showing the amount of relative motion
    // being applied.
    private void UpdateFeedback(Model model, Vector3 motion, bool isSnapped) {
        // Get the dimension the motion is in.
        int dim = _faces[_activeFaceIndex].GetDimension();

        // Get the starting and end points in stage coordinates. The motion
        // vector is already in stage coordinates.
        Matrix4x4 osm = UT.GetObjectToStageMatrix(gameObject);
        Vector3 p0 = osm.MultiplyPoint3x4(Vector3.zero);

        // Compute the direction of motion in stage coordinates. This has to
        // be correct even when the motion vector has zero length.
        Vector3 motionDir = osm.MultiplyVector(UT.GetAxis(dim)).normalized;
        float sign = Mathf.Sign(Vector3.Dot(motionDir, motion));

        // Update the feedback object.
        _feedback.SetColor(GetFeedbackColor(dim, isSnapped));
        _feedback.SpanLength(p0, motionDir, sign * motion.magnitude);
    }
}

}  // namespace MakerVR
