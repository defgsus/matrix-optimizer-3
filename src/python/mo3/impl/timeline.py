

class Timeline:
    """
    _CPP_:
        MATH::TimelineNd* tl;
    _CPP_(NEW):
        tl = nullptr;
    _CPP_(FREE):
        if (tl)
            tl->releaseRef("py free");
    _CPP_(COPY):
        if (tl)
            copy->tl = new MATH::TimelineNd(*tl);

    _CPP_(DEF):
        //PyObject* toPython(MO::MATH::TimelineNd*);
        bool py_get_time_and_vec(
            PyObject* args_, double* time, MATH::TimelineNd::ValueType* vec);
        bool py_get_time_and_vec_and_pointtype(
                PyObject* args_, double* time, MATH::TimelineNd::ValueType* vec,
                int* ptype);
    _CPP_(IMPL):
        /*
        PyObject* toPython(MO::MATH::TimelineNd* tl)
        {
            ret = $NEW(Timeline);
            ret->tl = tl;
            if (tl)
                tl->addRef("py create");
            return (PyObject*)ret;
        }

        bool py_get_time_and_vec(
            PyObject* args_, double* time, MATH::TimelineNd::ValueType* vec)
        {
            if (vec->numDimensions() == 1)
            {
                if (PyArg_ParseTuple(args_, "dd", time, &(*vec)[0]))
                    return true;
            }
            else if (vec->numDimensions() == 2)
            {
                if (PyArg_ParseTuple(args_, "ddd", time, &(*vec)[0], &(*vec)[1]))
                    return true;
            }
            else if (vec->numDimensions() == 3)
            {
                if (PyArg_ParseTuple(args_, "dddd", time, &(*vec)[0], &(*vec)[1], &(*vec)[2]))
                    return true;
            }
            else if (vec->numDimensions() == 4)
            {
                if (PyArg_ParseTuple(args_, "ddddd", time, &(*vec)[0], &(*vec)[1], &(*vec)[2], &(*vec)[3]))
                    return true;
            }
            PyErr_Clear();
            PyObject * second;
            if (!PyArg_ParseTuple(args_, "dO", time, &second))
                return false;
            if (get_vector(second, vec->numDimensions(), &(*vec)[0]))
                return true;
            return false;
        }

        bool py_get_time_and_vec_and_pointtype(
                PyObject* args_, double* time, MATH::TimelineNd::ValueType* vec,
                int* ptype)
        {
            if (vec->numDimensions() == 1)
            {
                if (PyArg_ParseTuple(args_, "ddi", time, &(*vec)[0], ptype))
                    return true;
            }
            else if (vec->numDimensions() == 2)
            {
                if (PyArg_ParseTuple(args_, "dddi", time, &(*vec)[0], &(*vec)[1], ptype))
                    return true;
            }
            else if (vec->numDimensions() == 3)
            {
                if (PyArg_ParseTuple(args_, "ddddi",
                                     time, &(*vec)[0], &(*vec)[1], &(*vec)[2], ptype))
                    return true;
            }
            else if (vec->numDimensions() == 4)
            {
                if (PyArg_ParseTuple(args_, "dddddi",
                        time, &(*vec)[0], &(*vec)[1], &(*vec)[2], &(*vec)[3], ptype))
                    return true;
            }
            PyErr_Clear();
            PyObject * second;
            if (!PyArg_ParseTuple(args_, "dOi", time, &second, ptype))
                return false;
            if (get_vector(second, vec->numDimensions(), &(*vec)[0]))
                return true;
            return false;
        }*/

    """


    def __init__(self):
        """_CPP_:
            long l;
            arg1 = removeArgumentTuple(arg1);
            if (!expectFromPython(arg1, &l))
                return -1;
            self->tl = new MATH::TimelineNd(l);
            return 0;
        """
        pass

    def __str__(self):
        """_CPP_:
            MO__ASSERT_TL(self);
            return toPython(QString("Timeline(dim %1, points %2)")
                  .arg(self->tl->numDimensions())
                  .arg(self->tl->size()) );
        """
        pass

    @property
    def dimensions(self):
        """
        dimensions() -> int
        Returns the number of dimensions, e.g. the number of components per vector
        _CPP_:
            MO__ASSERT_TL(self);
            return toPython(self->tl->numDimensions());
        """
        return 0

    @property
    def size(self):
        """
        size() -> int
        Returns the number of points
        _CPP_:
            MO__ASSERT_TL(self);
            return toPython(self->tl->size());
        """
        return 0

    @property
    def start(self):
        """
        start() -> float
        The time of the first cue in seconds
        _CPP_:
            MO__ASSERT_TL(self);
            return toPython(self->tl->empty()
                            ? 0.0 : self->tl->getData().begin()->second.t);
        """
        return 0

    @property
    def end(self):
        """
        end() -> float
        The time of the last cue in seconds
        _CPP_:
            MO__ASSERT_TL(self);
            return toPython(self->tl->empty()
                            ? 0.0 : self->tl->getData().rbegin()->second.t);
        """
        return 0

    @property
    def length(self):
        """
        length() -> float
        The time between the first and last cue in seconds
        _CPP_:
            MO__ASSERT_TL(self);
            return toPython(self->tl->empty() ? 0.0
                        : self->tl->getData().rbegin()->second.t
                          - self->tl->getData().begin()->second.t);
        """
        return 0

    def as_set(self):
        """
        as_set() -> set\n"
        Returns a set containing the times of all timeline points
        _CPP_:
            MO__ASSERT_TL(self);
            auto set = PySet_New(NULL);
            if (!set)
            {
                setPythonError(PyExc_MemoryError, "Could not create set object");
                return NULL;
            }
            for (auto& i : self->tl->getData())
            {
                auto d = toPython(i.second.t);
                if (!d)
                {
                    Py_DECREF(set);
                    return NULL;
                }
                if (PySet_Add(set, d) < 0)
                {
                    Py_DECREF(set);
                    return NULL;
                }
            }
            return set;
        """
        pass

    def copy(self):
        """
        copy() -> Timeline
        Returns a new instance with the same data
        _CPP_:
            return (PyObject*)$COPY(self);
        """
        pass

    def value(self, t):
        """
        value(f) -> f | list
        Returns the value at the given time
        If dimension is 1, returns float.
        If dimension is >1, returns list of float
        _CPP_:
            MO__ASSERT_TL(self);
            double time;
            if (!fromPython(arg1, &time))
                return NULL;
            if (self->tl->numDimensions() == 1)
                return toPython(self->tl->get(time)[0]);
            return toPython(self->tl->get(time));
        """
        pass

    def derivative(self, t, r):
        """
        derivative(f, f) -> f | list
        Returns the derivative at the given time.
        The optional second argument is the time range to observe, initialized to 0.01
        If dimension is 1, returns float.
        If dimension is >1, returns list of float
        _CPP_:
            MO__ASSERT_TL(self);
            double time, range = 0.01;
            if (!PyArg_ParseTuple(arg1, "d|d", &time, &range))
                return NULL;
            range = .5001 * std::max(range, MATH::TimelineNd::timeQuantum());
            if (self->tl->numDimensions() == 1)
                return toPython(self->tl->getDerivative(time, range)[0]);
            return toPython(self->tl->getDerivative(time, range));
        """
        pass

    def get_timeline(self, i):
        """
        get_timeline(i) -> Timeline | None
        Returns the i'th dimension as one-dimensional timeline.
        _CPP_:
            MO__ASSERT_TL(self);
            long idx = PyLong_AsLong(arg1);
            if (idx < 0 || (size_t)idx >= self->tl->numDimensions())
            {
                setPythonError(PyExc_IndexError, SStream() << "Dimension out of range "
                            << idx << "/" << self->tl->numDimensions());
                return NULL;
            }
            auto tl1 = self->tl->getTimelineNd(idx, 1);
            auto ret = reinterpret_cast<PyObject*>(toPython(tl1));
            tl1->releaseRef("py create finish");
            return ret;
        """
        pass

    def set_dimensions(self, i):
        """
        set_dimensions(long) -> None
        Sets the number of dimensions which must be at least one.
        Each point in the timeline is affected. If the dimensionality grows,
        new data is initialized to zero.
        _CPP_:
            MO__ASSERT_TL(self);
            long num;
            if (!PyArg_ParseTuple(arg1, "l", &num))
                return NULL;
            if (num<1 || num>4)
            {
                setPythonError(PyExc_TypeError,
                    SStream() << "number of dimensions out of range 1<=" << num << "<=4");
                return NULL;
            }
            self->tl->setDimensions(num);
            if (num<1 || num>4)
            self->tl->setDimensions(num);
            Py_RETURN_NONE;
        """
        pass

    def add(self, t, v):
        """
        add(float, vec) -> self
        add(float, vec, int) -> self
        Adds a value/vector at the given time.
        The vector size must fit the dimension of the timeline data.
        The third argument can be on of the Timeline point types,
        CONSTANT, LINEAR, SMOOTH, SYMMETRIC, SYMMETRIC_USER, HERMITE,
        SPLINE4, SPLINE6
        _CPP_:
            MO__ASSERT_TL(self);
            /*
            double time;
            MATH::TimelineNd::ValueType
                    val(self->tl->numDimensions(), MATH::TimelineNd::ValueType::NoInit);
            int ptype = MATH::TimelinePoint::DEFAULT;
            if (!py_get_time_and_vec_and_pointtype(arg1, &time, &val, &ptype))
            {
                //MO_DEBUG("TODO Python Timeline: PTYPE not retrieved");
                PyErr_Clear();
                if (!py_get_time_and_vec(arg1, &time, &val))
                    return NULL;
            }
            self->tl->add(time, val, (MATH::TimelinePoint::Type)ptype);
            */
            Py_RETURN_SELF;
        """
        pass