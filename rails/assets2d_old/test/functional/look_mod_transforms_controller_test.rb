require File.dirname(__FILE__) + '/../test_helper'
require 'look_mod_transforms_controller'

# Re-raise errors caught by the controller.
class LookModTransformsController; def rescue_action(e) raise e end; end

class LookModTransformsControllerTest < Test::Unit::TestCase
  fixtures :look_mod_transforms

  def setup
    @controller = LookModTransformsController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new

    @first_id = look_mod_transforms(:first).id
  end

  def test_index
    get :index
    assert_response :success
    assert_template 'list'
  end

  def test_list
    get :list

    assert_response :success
    assert_template 'list'

    assert_not_nil assigns(:look_mod_transforms)
  end

  def test_show
    get :show, :id => @first_id

    assert_response :success
    assert_template 'show'

    assert_not_nil assigns(:look_mod_transform)
    assert assigns(:look_mod_transform).valid?
  end

  def test_new
    get :new

    assert_response :success
    assert_template 'new'

    assert_not_nil assigns(:look_mod_transform)
  end

  def test_create
    num_look_mod_transforms = LookModTransform.count

    post :create, :look_mod_transform => {}

    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_equal num_look_mod_transforms + 1, LookModTransform.count
  end

  def test_edit
    get :edit, :id => @first_id

    assert_response :success
    assert_template 'edit'

    assert_not_nil assigns(:look_mod_transform)
    assert assigns(:look_mod_transform).valid?
  end

  def test_update
    post :update, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'show', :id => @first_id
  end

  def test_destroy
    assert_nothing_raised {
      LookModTransform.find(@first_id)
    }

    post :destroy, :id => @first_id
    assert_response :redirect
    assert_redirected_to :action => 'list'

    assert_raise(ActiveRecord::RecordNotFound) {
      LookModTransform.find(@first_id)
    }
  end
end
